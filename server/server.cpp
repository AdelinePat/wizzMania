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
