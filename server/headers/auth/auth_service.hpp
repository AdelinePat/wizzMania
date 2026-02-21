#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

// #include "auth.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
// using WSConn = crow::websocket::connection*;
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#include "utils.hpp"

#define JWT_DISABLE_PICOJSON
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include <nlohmann/json.hpp>

class AuthService {
  //   Database& db;

 private:
  static std::string SECRET_KEY;

 public:
  //   explicit AuthService(Database& db) : db(db) {}
  static std::string create_token(
      const std::chrono::system_clock::time_point& now,
      const std::chrono::system_clock::time_point& expiration, int64_t id_user);
  //  void verify_token(std::string& token);
  static jwt::decoded_jwt<jwt::traits::nlohmann_json> decode(
      const std::string& token);
  static jwt::verifier<jwt::default_clock, jwt::traits::nlohmann_json>
  create_verifier();
};

#endif