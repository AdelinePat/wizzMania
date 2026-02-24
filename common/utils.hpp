#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class Utils {
 public:
  // Delete constructor to prevent instantiation
  Utils() = delete;

  // Static utility methods
  static const std::string get_env_var(const std::string& name,
                                       const std::string& default_val = "");
  static uint16_t get_server_port();
  static std::string get_timestamp();
};

#endif