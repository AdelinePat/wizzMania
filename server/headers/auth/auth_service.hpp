#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

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
 private:
  //   WebSocketManager& ws_manager;
  std::string SECRET_KEY = Utils::get_env_var("SECRET_KEY", "default_secret");

  jwt::decoded_jwt<jwt::traits::nlohmann_json> decode(const std::string& token);
  jwt::verifier<jwt::default_clock, jwt::traits::nlohmann_json>
  create_verifier();
  std::optional<int64_t> get_validated_id_user(const std::string& token);

 public:
  explicit AuthService() {}
  std::string create_token(
      const std::chrono::system_clock::time_point& now,
      const std::chrono::system_clock::time_point& expiration, int64_t id_user);
  int64_t validate_token(const std::string& token);
};

#endif