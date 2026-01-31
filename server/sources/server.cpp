#include "server.hpp"
#include "helpers.hpp"

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
int main() {
    std::cout << "========================================\n";
    std::cout << " WizzMania Server - Crow HTTP Server \n";
    std::cout << "========================================\n";
    std::cout.setf(std::ios::unitbuf);

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

    std::cout << "[INFO] Server starting on port: " << port << "\n";

    crow::SimpleApp app;

    // --------------------
    // POST /login
    // --------------------
    CROW_ROUTE(app, "/login").methods("POST"_method)
    ([](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        int64_t user_id = check_user_credentials(username, password);
        if (user_id < 0) return crow::response(401, "Invalid credentials");

        crow::json::wvalue res;
        res["status"] = "ok";
        res["user_id"] = user_id;
        return crow::response(res);
    });

    // --------------------
    // WebSocket /ws
    // --------------------
    CROW_ROUTE(app, "/ws").websocket(&app)
    .onopen([&]([[maybe_unused]] crow::websocket::connection& conn){
        // Do nothing on open; wait for first client message
    })
    .onclose([&]([[maybe_unused]] crow::websocket::connection& conn,
                 [[maybe_unused]] const std::string& reason,
                 [[maybe_unused]] uint16_t status_code){
        std::lock_guard<std::mutex> lock(ws_mutex);
        WSConn c = &conn;
        auto it = socket_to_user.find(c);
        if (it != socket_to_user.end()) {
            int64_t user_id = it->second;
            user_sockets[user_id].erase(c);
            if (user_sockets[user_id].empty())
                user_sockets.erase(user_id);
            socket_to_user.erase(c);
        }
    })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool){
        auto msg = crow::json::load(data);
        if (!msg) return;

        std::string event = msg["event"].s();

        if (event == "init") {
            int64_t user_id = msg["user_id"].i();

            {
                std::lock_guard<std::mutex> lock(ws_mutex);
                WSConn c = &conn;
                user_sockets[user_id].insert(c);
                socket_to_user[c] = user_id;
            }

            // Get channels (must be a wvalue array)
            crow::json::wvalue channels_array = get_channels_for_user(user_id);

            crow::json::wvalue res;
            res["event"] = "channel_list";

            // Properly assign array
            res["channels"] = crow::json::wvalue::list();
            for (size_t i = 0; i < channels_array.size(); ++i) {
                res["channels"][i] = std::move(channels_array[i]);
            }

            conn.send_text(res.dump());
        }

        // Future: handle other events like send_message
    });

    app.port(port).multithreaded().run();
    return 0;
}
