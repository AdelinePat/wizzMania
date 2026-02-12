#include "server.hpp"
// #include "helpers.hpp"

using WSConn = crow::websocket::connection*;

// --------------------
// WebSocket connection maps
// --------------------
std::unordered_map<int64_t, std::unordered_set<WSConn>> user_sockets;
std::unordered_map<WSConn, int64_t> socket_to_user;
std::mutex ws_mutex;

// --------------------
// Main
// --------------------

#include <iostream>

#include "auth.hpp"

int main() {
  std::cout << "========================================\n";
  std::cout << " WizzMania Server Starting...\n";
  std::cout << "========================================\n";

  crow::SimpleApp app;

  // ===== POST /login endpoint =====
  CROW_ROUTE(app, "/login")
      .methods("POST"_method)([](const crow::request& req) {
        std::cout << "[LOGIN] Received login request\n";

        // Parse JSON body
        auto json_body = crow::json::load(req.body);
        if (!json_body) {
          std::cout << "[LOGIN] Invalid JSON\n";
          crow::json::wvalue error;
          error["success"] = false;
          error["message"] = "Invalid JSON";
          return crow::response(400, error);
        }

        // Extract username and password
        std::string username = json_body["username"].s();
        std::string password = json_body["password"].s();

        std::cout << "[LOGIN] Attempting login for user: " << username << "\n";

        // Check credentials (using helpers.cpp)
        int64_t id_user = check_user_credentials(username, password);

        if (id_user < 0) {
          std::cout << "[LOGIN] Invalid credentials for: " << username << "\n";
          crow::json::wvalue error;
          error["success"] = false;
          error["message"] = "Invalid username or password";
          return crow::response(401, error);
        }

        // Generate JWT token
        std::string token = Auth::generateToken(id_user);
        std::cout << "[LOGIN] Login successful! User ID: " << id_user << "\n";
        std::cout << "[LOGIN] Token generated: " << token.substr(0, 20)
                  << "...\n";

        // Return success response
        crow::json::wvalue response;
        response["success"] = true;
        response["token"] = token;
        response["id_user"] = id_user;
        response["username"] = username;

        return crow::response(200, response);
      });

  // // ===== POST /ws/connect endpoint (simulates WebSocket handshake) =====
  // CROW_ROUTE(app, "/ws/connect")
  // .methods("POST"_method)
  // ([](const crow::request& req) {
  //     std::cout << "[WS] Received WebSocket connection request\n";

  //     // Get token from Authorization header
  //     std::string auth_header = req.get_header_value("Authorization");

  //     if (auth_header.empty()) {
  //         std::cout << "[WS] No Authorization header provided\n";
  //         crow::json::wvalue error;
  //         error["success"] = false;
  //         error["message"] = "Missing Authorization header";
  //         return crow::response(401, error);
  //     }

  //     // Extract token from "Bearer <token>" format
  //     std::string token;
  //     if (auth_header.substr(0, 7) == "Bearer ") {
  //         token = auth_header.substr(7);  // Remove "Bearer " prefix
  //     } else {
  //         token = auth_header;  // If no "Bearer " prefix, use as-is
  //     }

  //     std::cout << "[WS] Validating token: " << token.substr(0, 20) <<
  //     "...\n";

  //     // Validate token
  //     auto id_user = Auth::validateToken(token);

  //     if (!id_user.has_value()) {
  //         std::cout << "[WS] Invalid or expired token\n";
  //         crow::json::wvalue error;
  //         error["success"] = false;
  //         error["message"] = "Invalid or expired token";
  //         return crow::response(401, error);
  //     }

  //     std::cout << "[WS] Token valid! User ID: " << id_user.value() << "\n";
  //     std::cout << "[WS] WebSocket connection authorized (simulated)\n";

  //     // Return success response
  //     crow::json::wvalue response;
  //     response["success"] = true;
  //     response["message"] = "Token OK, WebSocket opened (simulated)";
  //     response["id_user"] = id_user.value();

  //     return crow::response(200, response);
  // });

  // ===== Real WebSocket endpoint =====
  CROW_ROUTE(app, "/ws")
      .websocket(&app)  // ← FIX: Pass app pointer
      .onopen([&]([[maybe_unused]] crow::websocket::connection& conn) {
        std::cout
            << "[WS] New WebSocket connection opened (waiting for auth)\n";
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason,
                   uint16_t status_code) {
        std::lock_guard<std::mutex> lock(ws_mutex);

        WSConn c = &conn;
        auto it = socket_to_user.find(c);

        if (it != socket_to_user.end()) {
          int64_t id_user = it->second;
          std::cout << "[WS] User " << id_user
                    << " disconnected. Reason: " << reason
                    << " (code: " << status_code << ")\n";

          user_sockets[id_user].erase(c);
          if (user_sockets[id_user].empty()) {
            user_sockets.erase(id_user);
          }
          socket_to_user.erase(c);
        } else {
          std::cout << "[WS] Unknown connection closed\n";
        }
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data,
                     [[maybe_unused]] bool is_binary) {
        std::cout << "[WS] Received message: " << data << "\n";

        WSConn c = &conn;

        // Check if already authenticated
        {
          std::lock_guard<std::mutex> lock(ws_mutex);
          if (socket_to_user.find(c) != socket_to_user.end()) {
            int64_t id_user = socket_to_user[c];
            std::cout << "[WS] Message from authenticated user " << id_user
                      << "\n";

            crow::json::wvalue response;
            response["type"] = "echo";
            response["message"] = "Received: " + data;
            conn.send_text(response.dump());
            return;
          }
        }

        // First message must be authentication
        auto json_msg = crow::json::load(data);
        if (!json_msg) {
          std::cout << "[WS] Invalid JSON in first message\n";
          conn.close("Invalid JSON");
          return;
        }

        if (!json_msg.has("token")) {
          std::cout << "[WS] No token in first message\n";
          conn.close("Missing token");
          return;
        }

        std::string token = json_msg["token"].s();
        std::cout << "[WS] Authenticating with token: " << token.substr(0, 20)
                  << "...\n";

        auto id_user = Auth::validateToken(token);

        if (!id_user.has_value()) {
          std::cout << "[WS] Invalid token, closing connection\n";
          conn.close("Invalid token");
          return;
        }

        // Token valid! Add to maps
        {
          std::lock_guard<std::mutex> lock(ws_mutex);
          user_sockets[id_user.value()].insert(c);
          socket_to_user[c] = id_user.value();
        }

        std::cout << "[WS] ✅ User " << id_user.value()
                  << " authenticated and connected!\n";

        // Send welcome message
        crow::json::wvalue welcome;
        welcome["type"] = "connected";
        welcome["message"] = "WebSocket connected successfully";
        welcome["id_user"] = id_user.value();
        conn.send_text(welcome.dump());
      });

  // ===== Start server =====
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

//         int64_t id_user = check_user_credentials(username, password);
//         if (id_user < 0) return crow::response(401, "Invalid credentials");

//         crow::json::wvalue res;
//         res["status"] = "ok";
//         res["id_user"] = id_user;
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
//             int64_t id_user = it->second;
//             user_sockets[id_user].erase(c);
//             if (user_sockets[id_user].empty())
//                 user_sockets.erase(id_user);
//             socket_to_user.erase(c);
//         }
//     })
//     .onmessage([&](crow::websocket::connection& conn, const std::string&
//     data, bool){
//         auto msg = crow::json::load(data);
//         if (!msg) return;

//         std::string event = msg["event"].s();

//         if (event == "init") {
//             int64_t id_user = msg["id_user"].i();

//             {
//                 std::lock_guard<std::mutex> lock(ws_mutex);
//                 WSConn c = &conn;
//                 user_sockets[id_user].insert(c);
//                 socket_to_user[c] = id_user;
//             }

//             // Get channels (must be a wvalue array)
//             crow::json::wvalue channels_array =
//             get_channels_for_user(id_user);

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
