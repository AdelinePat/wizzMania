#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;


// Handle an HTTP request and send a response
void handle_request(tcp::socket& socket) {
  beast::flat_buffer buffer;
  http::request<http::string_body> req;

  try {
    http::read(socket, buffer, req);

    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "WizzMania-Beast");
    res.set(http::field::content_type, "text/plain");

    if (req.target() == "/") {
      res.body() = "Hello from Beast !!!\n";
    } else if (req.target() == "/florence") {
      res.body() = "Hello to Florence from Beast !!!!\n";
    } else {
      res.result(http::status::not_found);
      res.body() = "Route not found\n";
    }

    res.prepare_payload();
    http::write(socket, res);
  } catch (std::exception& e) {
    std::cerr << "[ERROR] Request handling failed: " << e.what() << "\n";
  }
}

int main() {
  // Disable buffering entirely
  std::cout << "========================================\n";
  std::cout << " WizzMania Server - Beast HTTP Server \n";
  std::cout << "========================================\n";
  std::cout.setf(std::ios::unitbuf);

  const char* portStr = std::getenv("SERVER_PORT");
  uint16_t port = 8888;  // default port

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
  std::cout << "[INFO] Build & runtime environment OK\n";
  std::cout << "[INFO] Waiting...\n";

  try {
    net::io_context ioc{1};
    tcp::acceptor acceptor{ioc, tcp::endpoint{tcp::v4(), port}};

    while (true) {
      tcp::socket socket{ioc};
      acceptor.accept(socket);

      // Launch a thread per connection (simplest multithreading)
      std::thread([s = std::move(socket)]() mutable {
        handle_request(s);
      }).detach();
    }
  } catch (std::exception& e) {
    std::cerr << "[FATAL] Server failed: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return 0;
}

CROW_ROUTE(app, "/ws").websocket()
.onopen([&](crow::websocket::connection& conn){
    // Get token or user_id from query parameter
    auto user_id_str = conn.get_url_param("user_id");
    if (!user_id_str) {
        conn.close("Missing user_id");
        return;
    }

    int64_t user_id = std::stoll(user_id_str);

    // Save connection
    {
        std::lock_guard<std::mutex> lock(ws_mutex);
        WSConn c = &conn;
        user_sockets[user_id].insert(c);
        socket_to_user[c] = user_id;
    }

    // Fetch user's channels from DB
    auto channels = get_channels_for_user(user_id);

    // Send channel list to client
    crow::json::wvalue res;
    res["event"] = "channel_list";
    res["channels"] = channels; // channels should be an array of objects
    conn.send_text(crow::json::dump(res));
})
.onclose([&](crow::websocket::connection& conn, const std::string&){
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
    // For now, no messages handled yet
});
