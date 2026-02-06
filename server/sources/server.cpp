#include "server.hpp"

using WSConn = crow::websocket::connection*;

// --------------------
// WebSocket connection maps
// --------------------
std::unordered_map<int64_t, std::unordered_set<WSConn>> user_sockets;
std::unordered_map<WSConn, int64_t> socket_to_user;
std::mutex ws_mutex;

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

  crow::App<CORS> app;

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
      .methods("POST"_method)([](const crow::request& req) {
        std::cout << "[LOGIN] Received login request\n";

        auto json_body = crow::json::load(req.body);
        if (!json_body) {
          std::cout << "[LOGIN] Invalid JSON\n";
          crow::json::wvalue error;
          error["success"] = false;
          error["message"] = "Invalid JSON";
          return crow::response(400, error);
        }

        std::string username = json_body["username"].s();
        std::string password = json_body["password"].s();

        std::cout << "[LOGIN] Attempting login for user: " << username << "\n";

        int64_t user_id = check_user_credentials(username, password);

        if (user_id < 0) {
          std::cout << "[LOGIN] Invalid credentials for: " << username << "\n";
          crow::json::wvalue error;
          error["success"] = false;
          error["message"] = "Invalid username or password";
          return crow::response(401, error);
        }

        std::string token = Auth::generateToken(user_id);
        std::cout << "[LOGIN] Login successful! User ID: " << user_id << "\n";
        std::cout << "[LOGIN] Token generated: " << token.substr(0, 20)
                  << "...\n";

        crow::json::wvalue response;
        response["success"] = true;
        response["token"] = token;
        response["user_id"] = user_id;
        response["username"] = username;

        return crow::response(200, response);
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
        std::lock_guard<std::mutex> lock(ws_mutex);

        auto it = socket_to_user.find(&conn);
        if (it != socket_to_user.end()) {
          int64_t user_id = it->second;
          std::cout << "[WS] User " << user_id
                    << " disconnected. Reason: " << reason << "\n";

          user_sockets[user_id].erase(&conn);
          if (user_sockets[user_id].empty()) {
            user_sockets.erase(user_id);
          }
          socket_to_user.erase(&conn);
        } else {
          std::cout << "[WS] Unauthenticated connection closed. Reason: "
                    << reason << "\n";
        }
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data,
                     [[maybe_unused]] bool is_binary) {
        // Check if already authenticated by looking in socket_to_user map
        bool is_authenticated = false;
        int64_t user_id = -1;

        {
          std::lock_guard<std::mutex> lock(ws_mutex);
          auto it = socket_to_user.find(&conn);
          if (it != socket_to_user.end()) {
            is_authenticated = true;
            user_id = it->second;
          }
        }

        // If not authenticated, expect authentication message
        if (!is_authenticated) {
          std::cout << "[WS] Received authentication message\n";

          auto json_msg = crow::json::load(data);
          if (!json_msg || !json_msg.has("token")) {
            std::cout << "[WS] Missing or invalid token in auth message\n";
            conn.close("Missing or invalid token");
            return;
          }

          std::string token = json_msg["token"].s();
          std::cout << "[WS] Validating token: " << token.substr(0, 20)
                    << "...\n";

          auto validated_user_id = Auth::validateToken(token);

          if (!validated_user_id.has_value()) {
            std::cout << "[WS] Invalid token, closing connection\n";
            conn.close("Invalid token");
            return;
          }

          std::cout << "[WS] ✅ User " << validated_user_id.value()
                    << " authenticated successfully!\n";

          // Store connection in maps (this marks it as authenticated)
          {
            std::lock_guard<std::mutex> lock(ws_mutex);
            user_sockets[validated_user_id.value()].insert(&conn);
            socket_to_user[&conn] = validated_user_id.value();
          }

          // Send confirmation to client
          crow::json::wvalue welcome;
          welcome["type"] = "connected";
          welcome["message"] = "WebSocket authenticated successfully";
          welcome["user_id"] = validated_user_id.value();
          conn.send_text(welcome.dump());

          return;
        }

        // Handle regular messages from authenticated users
        std::cout << "[WS] Message from user " << user_id << ": " << data
                  << "\n";

        // Echo the message back for now (you can customize this)
        crow::json::wvalue response;
        response["type"] = "echo";
        response["message"] = data;
        response["user_id"] = user_id;
        conn.send_text(response.dump());
      });

  uint16_t port = 8888;
  std::cout << "[INFO] Server listening on port " << port << "\n";
  std::cout << "========================================\n";

  app.port(port).multithreaded().run();

  return 0;
}

// int main() {
//     std::cout << "========================================\n";
//     std::cout << " WizzMania Server - Crow HTTP Server \n";
//     std::cout << "========================================\n";
//     std::cout.setf(std::ios::unitbuf);

//     const char* portStr = std::getenv("SERVER_PORT");
//     uint16_t port = 8888;

//     if (!portStr) {
//         std::cout << "[WARN] SERVER_PORT not set. Using default 8888\n";
//         portStr = "8888";
//     }

//     try {
//         int temp = std::stoi(portStr);
//         if (temp > 0 && temp <= 65535)
//             port = static_cast<uint16_t>(temp);
//         else
//             std::cerr << "[WARN] SERVER_PORT out of range, using default\n";
//     } catch (...) {
//         std::cerr << "[WARN] Invalid SERVER_PORT, using default\n";
//     }

//     std::cout << "[INFO] Server starting on port: " << port << "\n";

//     crow::SimpleApp app;

//     // --------------------
//     // POST /login
//     // --------------------
//     CROW_ROUTE(app, "/login").methods("POST"_method)
//     ([](const crow::request& req){
//         auto body = crow::json::load(req.body);
//         if (!body) return crow::response(400, "Invalid JSON");

//         std::string username = body["username"].s();
//         std::string password = body["password"].s();

//         int64_t user_id = check_user_credentials(username, password);
//         if (user_id < 0) return crow::response(401, "Invalid credentials");

//         crow::json::wvalue res;
//         res["status"] = "ok";
//         res["user_id"] = user_id;
//         return crow::response(res);
//     });

//     // --------------------
//     // WebSocket /ws
//     // --------------------
//     CROW_ROUTE(app, "/ws").websocket(&app)
//     .onopen([&]([[maybe_unused]] crow::websocket::connection& conn){
//         // Do nothing on open; wait for first client message
//     })
//     .onclose([&]([[maybe_unused]] crow::websocket::connection& conn,
//                  [[maybe_unused]] const std::string& reason,
//                  [[maybe_unused]] uint16_t status_code){
//         std::lock_guard<std::mutex> lock(ws_mutex);
//         WSConn c = &conn;
//         auto it = socket_to_user.find(c);
//         if (it != socket_to_user.end()) {
//             int64_t user_id = it->second;
//             user_sockets[user_id].erase(c);
//             if (user_sockets[user_id].empty())
//                 user_sockets.erase(user_id);
//             socket_to_user.erase(c);
//         }
//     })
//     .onmessage([&](crow::websocket::connection& conn, const std::string&
//     data, bool){
//         auto msg = crow::json::load(data);
//         if (!msg) return;

//         std::string event = msg["event"].s();

//         if (event == "init") {
//             int64_t user_id = msg["user_id"].i();

//             {
//                 std::lock_guard<std::mutex> lock(ws_mutex);
//                 WSConn c = &conn;
//                 user_sockets[user_id].insert(c);
//                 socket_to_user[c] = user_id;
//             }

//             // Get channels (must be a wvalue array)
//             crow::json::wvalue channels_array =
//             get_channels_for_user(user_id);

//             crow::json::wvalue res;
//             res["event"] = "channel_list";

//             // Properly assign array
//             res["channels"] = crow::json::wvalue::list();
//             for (size_t i = 0; i < channels_array.size(); ++i) {
//                 res["channels"][i] = std::move(channels_array[i]);
//             }

//             conn.send_text(res.dump());
//         }

//         // Future: handle other events like send_message
//     });

//     app.port(port).multithreaded().run();
//     return 0;
// }
