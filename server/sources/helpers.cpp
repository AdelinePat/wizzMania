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

std::string get_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&now_time), "%Y-%m-%d %H:%M:%S");
  std::string timestamp = oss.str();
  return timestamp;
}

ServerSend::Message create_message(int64_t id_message, int64_t id_user,
                                   const std::string& body,
                                   const std::string& timestamp) {
  ServerSend::Message message;
  message.id_message = id_message;
  message.id_sender = id_user;
  message.body = body;
  message.timestamp = timestamp;
  message.is_system = id_user == 1;
  return message;
}