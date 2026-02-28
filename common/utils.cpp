#include "utils.hpp"

const std::string Utils::get_env_var(const std::string& name,
                                     const std::string& default_val) {
  const char* value = std::getenv(name.c_str());
  if (!value) {
    if (!default_val.empty())
      std::cout << "[WARN] " << name
                << " not set. Using default: " << default_val << "\n";
    else
      std::cerr << "[WARN] " << name << " not set and no default provided.\n";
    return default_val;
  }
  return std::string(value);
}

uint16_t Utils::get_server_port() {
  //   const char* portStr = std::getenv("SERVER_PORT");
  // const char* portStr = Utils::get_env_var("SERVER_PORT").c_str();
  std::string portStr = Utils::get_env_var("SERVER_PORT");
  uint16_t port = 8888;

  if (portStr.empty()) {
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

// Get rid of white space at the begining and end of string, if input empty
// throws BadInput error
std::string Utils::trim(const std::string& input) {
  size_t start = input.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) {
    throw BadInput("Field can't be empty");
  }
  size_t end = input.find_last_not_of(" \t\n\r");
  return input.substr(start, end - start + 1);  // why +1 ?
}

// Checks only alphanumeric character and underscore + lowercase the string, if
// something goes wrong throw BadInput error
std::string Utils::clean_username(const std::string& username) {
  for (char character : username) {
    if (!std::isalnum(character) && character != '_') {
      throw BadInput(std::string("Invalid character '") + character +
                     "' in username. "
                     "Only letters, numbers and underscores are allowed.");
    }
  }
  std::string result = username;
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);

  return result;
}

// Only printable ASCII allowed
bool Utils::is_valid_password_chars(const std::string& s) {
  return std::all_of(s.begin(), s.end(),
                     [](unsigned char c) { return c > 32 && c <= 126; });
  // ASCII 32 = white space, I won't allow password with whitespace within it
}

bool Utils::is_valid_email(const std::string& email) {
  static const std::regex pattern(
      R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)");
  return std::regex_match(email, pattern);
}

bool Utils::is_valid_password(const std::string& password) {
  if (password.size() < 8) return false;
  if (!is_valid_password_chars(password)) return false;

  static const std::regex has_upper("[A-Z]");
  static const std::regex has_lower("[a-z]");
  static const std::regex has_digit("[0-9]");
  static const std::regex has_special(R"([!@#$%^&*\-_=+;:,.<>?/])");

  return std::regex_search(password, has_upper) &&
         std::regex_search(password, has_lower) &&
         std::regex_search(password, has_digit) &&
         std::regex_search(password, has_special);
}