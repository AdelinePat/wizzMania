#include "helpers.hpp"

uint16_t get_server_port() {
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

  return port;
}