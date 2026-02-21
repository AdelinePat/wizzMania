#include "utils.hpp"

const std::string Utils::get_env_var(const std::string& name, const std::string& default_val) {
    const char* value = std::getenv(name.c_str());
    if (!value) {
        if (!default_val.empty())
            std::cout << "[WARN] " << name << " not set. Using default: " << default_val << "\n";
        else
            std::cerr << "[WARN] " << name << " not set and no default provided.\n";
        return default_val;
    }
    return std::string(value);
}

uint16_t Utils::get_server_port() {
//   const char* portStr = std::getenv("SERVER_PORT");
  const char* portStr = Utils::get_env_var("SERVER_PORT").c_str();
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

std::string Utils::get_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&now_time), "%Y-%m-%dT%H:%M:%S") << "Z";
  std::string timestamp = oss.str();
  return timestamp;
}