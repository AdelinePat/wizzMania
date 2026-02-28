#include "auth_service.hpp"

// std::string AuthService::SECRET_KEY =
//     Utils::get_env_var("SECRET_KEY", "default_secret");

std::string AuthService::create_token(
    const std::chrono::system_clock::time_point& now,
    const std::chrono::system_clock::time_point& expiration, int64_t id_user) {
  // Use basic_claim with nlohmann_json traits
  using claim = jwt::basic_claim<jwt::traits::nlohmann_json>;
  return jwt::create<jwt::traits::nlohmann_json>()
      .set_type("JWT")
      .set_issuer("wizzmania")
      .set_issued_at(now)
      .set_expires_at(expiration)
      .set_payload_claim("id_user", claim(std::to_string(id_user)))
      .sign(jwt::algorithm::hs256{SECRET_KEY});
}

jwt::verifier<jwt::default_clock, jwt::traits::nlohmann_json>
AuthService::create_verifier() {
  return jwt::verify<jwt::traits::nlohmann_json>()
      .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
      .with_issuer("wizzmania");
}

jwt::decoded_jwt<jwt::traits::nlohmann_json> AuthService::decode(
    const std::string& token) {
  return jwt::decode<jwt::traits::nlohmann_json>(token);
}

// private
std::optional<int64_t> AuthService::get_validated_id_user(const std::string& token) {
  try {
    auto verifier = this->create_verifier();
    auto decoded = this->decode(token);
    verifier.verify(decoded);

    std::string id_user_str = decoded.get_payload_claim("id_user").as_string();
    return std::stoll(id_user_str);

  } catch (const std::exception&) {
    return std::nullopt;
  }
}

int64_t AuthService::validate_token(const std::string& token) {
  std::optional<int64_t> validated_id_user = this->get_validated_id_user(token);

  if (!validated_id_user.has_value()) {
    // throw WsError("Invalid token");
    throw UnauthorizedError("Invalid token");
    // this->auth_error(conn, "Invalid token");
    // return;
  }
  return validated_id_user.value();
}