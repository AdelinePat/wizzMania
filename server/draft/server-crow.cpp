#include "server.hpp"

int main() {
  // Disable buffering entirely
  std::cout << "========================================\n";
  std::cout << " WizzMania Server - Test Crow WebSocket \n";
  std::cout << "========================================\n";
  std::cout.setf(std::ios::unitbuf);

  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() { return "Hello from Crow! \n"; });

  CROW_ROUTE(app, "/florence")([]() { return "Hello to Florence from Crow! \n"; });

  const char* portStr = std::getenv("SERVER_PORT");
  uint16_t port = 8888;  // default port

  if (!portStr) {
    std::cout << "[WARN] SERVER_PORT not set. Using default 8888\n";
    portStr = "8888";
  }

  int temp = std::stoi(portStr);

  if (temp > 0 && temp <= 65535) {
    port = static_cast<uint16_t>(temp);
  } else {
    std::cerr << "SERVER_PORT out of range, using default\n";
  }

  std::cout << "[INFO] Server starting on port: " << port << "\n";
  std::cout << "[INFO] Build & runtime environment OK\n";
  std::cout << "[INFO] Waiting..." << std::endl;

  
  app.port(port).multithreaded().run();

  //   while (true) {
  //     std::cout << "[HEARTBEAT] Server running...\n";
  //     std::this_thread::sleep_for(std::chrono::seconds(5));
  //   }

  return 0;
}
