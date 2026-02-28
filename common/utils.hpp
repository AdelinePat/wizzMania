#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

#include "client_exception.hpp"

class Utils {
 public:
  // Delete constructor to prevent instantiation
  Utils() = delete;

  // Static utility methods
  static const std::string get_env_var(const std::string& name,
                                       const std::string& default_val = "");
  static uint16_t get_server_port();
  static std::string get_timestamp();
  static std::string trim(const std::string& input);
  static std::string clean_username(const std::string& s);
  static bool is_valid_password_chars(const std::string& s);
  static bool is_valid_email(const std::string& email);
  static bool is_valid_password(const std::string& password);
};

#endif