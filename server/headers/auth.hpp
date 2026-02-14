#ifndef AUTH_H
#define AUTH_H

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#define JWT_DISABLE_PICOJSON
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include <nlohmann/json.hpp>

class Auth {
 public:
  static std::string generateToken(int64_t id_user);
  static std::optional<int64_t> validateToken(const std::string& token);

 private:
  static constexpr const char* SECRET_KEY =
      "your_secret_key_change_in_production";
  // static constexpr const char* SECRET_KEY = std::getenv("SECRET_KEY") ?
  // std::getenv("SECRET_KEY") : "your_secret_key_change_in_production";
};

#endif