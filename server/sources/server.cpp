#include "server.hpp"

// --------------------
// CORS Middleware
// --------------------
struct CORS {
  struct context {};

  void before_handle(crow::request&, crow::response& res, context&) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "*");
    res.add_header("Access-Control-Allow-Headers", "*");
    res.add_header("Access-Control-Expose-Headers", "*");
    res.add_header("Access-Control-Max-Age", "86400");
  }

  void after_handle(crow::request&, crow::response& res, context&) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "*");
    res.add_header("Access-Control-Allow-Headers", "*");
    res.add_header("Access-Control-Expose-Headers", "*");
  }
};

// --------------------
// Main
// --------------------
int main() {
  std::cout << "========================================\n";
  std::cout << " WizzMania Server Starting...\n";
  std::cout << "========================================\n";

  // =========================================
  // Initialize Server Port from environment
  // =========================================

  const char* portStr = std::getenv("SERVER_PORT");
  uint16_t port = 8888;

  if (!portStr) {
    std::cout << "[WARN] SERVER_PORT not set. Using default 8888\n";
    portStr = "8888";
  }

  try {
    int temp = std::stoi(portStr);
    if (temp > 0 && temp <= 65535)
      port = static_cast<uint16_t>(temp);
    else
      std::cerr << "[WARN] SERVER_PORT out of range, using default\n";
  } catch (...) {
    std::cerr << "[WARN] Invalid SERVER_PORT, using default\n";
  }

  // =========================================
  // Initialize Database Connection
  // =========================================
  std::string db_host =
      std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "mysql-db";
  std::string db_user =
      std::getenv("DB_USER") ? std::getenv("DB_USER") : "root";
  std::string db_pass =
      std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "root_password";
  std::string db_name =
      std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "wizzmania";

  Database db(db_host, db_user, db_pass, db_name);
  std::cout << "[Server] Database initialized successfully" << std::endl;

  crow::App<CORS> app;

  WebSocketManager ws_manager;
  MessageHandler msg_handler(db, ws_manager);

  // ===== OPTIONS for CORS preflight =====
  CROW_ROUTE(app, "/<path>")
      .methods("OPTIONS"_method)(
          [](const crow::request& req, crow::response& res, std::string path) {
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "*");
            res.add_header("Access-Control-Allow-Headers", "*");
            res.add_header("Access-Control-Expose-Headers", "*");
            res.code = 204;
            res.end();
          });

  // ===== POST /login endpoint =====
  CROW_ROUTE(app, "/login")
      .methods("POST"_method)([&db](const crow::request& req) {
        std::cout << "[LOGIN] Received login request\n";

        crow::json::rvalue json_body = crow::json::load(req.body);
        if (!json_body) {
          std::cout << "[LOGIN] Invalid JSON\n";
          AuthMessages::LoginResponse error_resp;
          error_resp.success = false;
          error_resp.message = "Invalid JSON";
          return crow::response(400, JsonHelpers::Auth::to_json(error_resp));
        }

        std::optional<AuthMessages::LoginRequest> login_req =
            JsonHelpers::Auth::parse_login_request(json_body);
        if (!login_req.has_value()) {
          std::cout << "[LOGIN] Missing required fields\n";
          AuthMessages::LoginResponse error_resp;
          error_resp.success = false;
          error_resp.message = "Missing username or password";
          return crow::response(400, JsonHelpers::Auth::to_json(error_resp));
        }

        std::cout << "[LOGIN] Attempting login for user: "
                  << login_req->username << "\n";

        int64_t user_id =
             db.verify_user(login_req->username, login_req->password);
        
        if (user_id < 0) {
          std::cout << "[LOGIN] Invalid credentials for: "
                    << login_req->username << "\n";
          AuthMessages::LoginResponse error_resp;
          error_resp.success = false;
          error_resp.message = "Invalid username or password";
          return crow::response(401, JsonHelpers::Auth::to_json(error_resp));
        }

        std::string token = Auth::generateToken(user_id);
        std::cout << "[LOGIN] Login successful! User ID: " << user_id << "\n";
        std::cout << "[LOGIN] Token generated: " << token.substr(0, 20)
                  << "...\n";

        AuthMessages::LoginResponse success_resp;
        success_resp.success = true;
        success_resp.message = "Login successful";
        success_resp.token = token;
        success_resp.user_id = user_id;
        success_resp.username = login_req->username;

        return crow::response(200, JsonHelpers::Auth::to_json(success_resp));
      });

  // ===== WebSocket endpoint =====
  CROW_ROUTE(app, "/ws")
      .websocket(&app)
      .onopen([&]([[maybe_unused]] crow::websocket::connection& conn) {
        std::cout
            << "[WS] New WebSocket connection opened (waiting for auth)\n";
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason,
                   uint16_t status_code) {
        std::optional<int64_t> user_id = ws_manager.get_user_id(&conn);
        if (user_id.has_value()) {
          std::cout << "[WS] User " << user_id.value()
                    << " disconnected. Reason: " << reason << "\n";
        } else {
          std::cout << "[WS] Unauthenticated connection closed. Reason: "
                    << reason << "\n";
        }

        ws_manager.remove_connection(&conn);

        // std::cout << "[WS] Online users: " <<
        // ws_manager.get_online_user_count()
        //           << "\n";
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data,
                     [[maybe_unused]] bool is_binary) {
        crow::json::rvalue json_msg = crow::json::load(data);
        if (!json_msg || !json_msg.has("type")) {
          std::cout << "[WS] Invalid JSON or missing 'type' field\n";
          conn.close("Invalid message format");
          return;
        }

        int type_int = json_msg["type"].i();
        WizzMania::MessageType msg_type =
            static_cast<WizzMania::MessageType>(type_int);

        // ===== AUTHENTICATION =====
        if (msg_type == WizzMania::MessageType::WS_AUTH) {
          if (ws_manager.is_authenticated(&conn)) {
            std::cout << "[WS] User already authenticated\n";
            return;
          }

          std::cout << "[WS] Processing authentication request\n";

          std::optional<::AuthMessages::WSAuthRequest> auth_req =
              JsonHelpers::Auth::parse_ws_auth_request(json_msg);
          if (!auth_req.has_value()) {
            std::cout << "[WS] Invalid auth request format\n";
            conn.close("Invalid authentication format");
            return;
          }

          // std::cout << "[WS] Validating token: "
          //           << auth_req->token.substr(0, 20) << "...\n";

          std::optional<int64_t> validated_user_id =
              Auth::validateToken(auth_req->token);

          if (!validated_user_id.has_value()) {
            std::cout << "[WS] Invalid token\n";
            conn.close("Invalid token");
            return;
          }

          std::cout << "[WS] ✅ User " << validated_user_id.value()
                    << " authenticated!\n";

          ws_manager.add_user(validated_user_id.value(), &conn);

          // {
          //   std::lock_guard<std::mutex> lock(ws_mutex);
          //   user_sockets[validated_user_id.value()].insert(&conn);
          //   socket_to_user[&conn] = validated_user_id.value();
          // }

          AuthMessages::WSAuthResponse auth_resp;
          auth_resp.type =
              WizzMania::MessageType::WS_AUTH_SUCCESS;  // Explicitly set type
          auth_resp.message = "Authentication successful";
          auth_resp.user_id = validated_user_id.value();
          conn.send_text(JsonHelpers::Auth::to_json(auth_resp).dump());

          return;
        }

        // ===== ALL OTHER MESSAGES REQUIRE AUTH =====
        // if (!is_authenticated) {
        //   std::cout << "[WS] Unauthenticated connection\n";
        //   conn.close("Authentication required");
        //   return;
        // }
        std::optional<int64_t> user_id_opt = ws_manager.get_user_id(&conn);
        if (!user_id_opt.has_value()) {
          std::cout << "[WS] Unauthenticated message attempt\n";
          conn.close("Authentication required");
          return;
        }

        // ===== ROUTE AUTHENTICATED MESSAGES =====
        int64_t user_id = user_id_opt.value();
        std::cout << "[WS] User " << user_id << " - type: " << type_int << "\n";

        switch (msg_type) {
          case WizzMania::MessageType::SEND_MESSAGE: {
            msg_handler.handle_send_message(conn, user_id, json_msg);
            break;
          }

          case WizzMania::MessageType::CREATE_CHANNEL: {
            auto req = JsonHelpers::ClientSend::parse_create_channel(json_msg);
            if (!req.has_value()) {
              ServerSend::ErrorResponse err;
              err.type = WizzMania::MessageType::ERROR;
              err.message = "Invalid CREATE_CHANNEL format";
              err.error_code = "INVALID_FORMAT";
              conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
              return;
            }

            std::cout << "[CHANNEL] User " << user_id << " creating with "
                      << req->participant_ids.size() << " participants\n";

            // TODO: Create channel

            break;
          }

          case WizzMania::MessageType::TYPING_START:
          case WizzMania::MessageType::TYPING_STOP: {
            auto req = JsonHelpers::ClientSend::parse_typing(json_msg);
            if (!req.has_value()) {
              ServerSend::ErrorResponse err;
              err.type = WizzMania::MessageType::ERROR;
              err.message = "Invalid TYPING format";
              err.error_code = "INVALID_FORMAT";
              conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
              return;
            }

            std::cout << "[TYPING] User " << user_id << " in channel "
                      << req->channel_id << ": "
                      << (req->is_typing ? "start" : "stop") << "\n";

            // TODO: Broadcast typing

            break;
          }

          case WizzMania::MessageType::LOGOUT: {
            auto req = JsonHelpers::Auth::parse_logout_request(json_msg);
            std::cout << "[LOGOUT] User " << user_id;
            if (req.has_value() && !req->reason.empty()) {
              std::cout << " (" << req->reason << ")";
            }
            std::cout << "\n";

            conn.close("User logout");
            break;
          }

          default:
            std::cout << "[WS] Unhandled type: " << type_int << "\n";
            ServerSend::ErrorResponse err;
            err.type = WizzMania::MessageType::ERROR;
            err.message = "Message type not implemented";
            err.error_code = "NOT_IMPLEMENTED";
            conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
            break;
        }
      });

  // uint16_t port = 8888;
  std::cout << "[INFO] Server listening on port " << port << "\n";
  std::cout << "========================================\n";

  app.port(port).multithreaded().run();

  return 0;
}