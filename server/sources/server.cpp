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

  uint16_t port = Utils::get_server_port();
  Database db;
  std::cout << "[Server] Database initialized successfully" << std::endl;

  crow::App<CORS> app;

  WebSocketManager ws_manager;
  MessageHandler msg_handler(db, ws_manager);
  // UserService user_service(db);
  UserController user_controller(db);
  MessageController message_controller(db, ws_manager);
  InvitationController invitation_controller(db, ws_manager);
  ChannelController channel_controller(db, ws_manager);

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
      .methods("POST"_method)([&user_controller](const crow::request& req) {
        return user_controller.login(req);
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
        std::optional<int64_t> id_user = ws_manager.get_id_user(&conn);
        if (id_user.has_value()) {
          std::cout << "[WS] User " << id_user.value()
                    << " disconnected. Reason: " << reason << "\n";
        } else {
          std::cout << "[WS] Unauthenticated connection closed. Reason: "
                    << reason << "\n";
        }
        ws_manager.remove_connection(&conn);
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
          // method in progress
          return msg_handler.authenticate_ws(conn, json_msg);
        }

        // ===== ALL OTHER MESSAGES REQUIRE AUTH =====
        std::optional<int64_t> id_user_opt = ws_manager.get_id_user(&conn);
        if (!id_user_opt.has_value()) {
          std::cout << "[WS] Unauthenticated message attempt\n";
          conn.close("Authentication required");
          return;
        }

        // ===== ROUTE AUTHENTICATED MESSAGES =====
        int64_t id_user = id_user_opt.value();
        std::cout << "[WS] User " << id_user << " - type: " << type_int << "\n";

        switch (msg_type) {
          case WizzMania::MessageType::SEND_MESSAGE: {
            message_controller.send_message(conn, id_user, json_msg);
            break;
          }

          case WizzMania::MessageType::REQUEST_CHANNEL_HISTORY: {
            message_controller.send_history(conn, id_user, json_msg);
            break;
          }

          case WizzMania::MessageType::ACCEPT_INVITATION: {
            // TO DO HTTP
            invitation_controller.accept_invitation(conn, id_user, json_msg);
            break;
          }

          case WizzMania::MessageType::REJECT_INVITATION: {
            // TO DO HTTP
            invitation_controller.reject_invitation(conn, id_user, json_msg);
            break;
          }

          case WizzMania::MessageType::CREATE_CHANNEL: {
            // TODO HTTP INSTEAD OF WS !! + check if creator invites themselves
            channel_controller.create_channel(conn, id_user, json_msg);
            break;
          }

            // case WizzMania::MessageType::TYPING_START:
            // case WizzMania::MessageType::TYPING_STOP: {
            //   auto req = JsonHelpers::ClientSend::parse_typing(json_msg);
            //   if (!req.has_value()) {
            //     ServerSend::ErrorResponse err;
            //     err.type = WizzMania::MessageType::ERROR;
            //     err.message = "Invalid TYPING format";
            //     err.error_code = "INVALID_FORMAT";
            //     conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
            //     return;
            //   }

            //   std::cout << "[TYPING] User " << id_user << " in channel "
            //             << req->id_channel << ": "
            //             << (req->is_typing ? "start" : "stop") << "\n";

            //   // TODO: Broadcast typing

            //   break;
            // }

          case WizzMania::MessageType::LOGOUT: {
            auto req = JsonHelpers::Auth::parse_logout_request(json_msg);
            std::cout << "[LOGOUT] User " << id_user;
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