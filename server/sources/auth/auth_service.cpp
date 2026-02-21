#include "auth_service.hpp"

std::string AuthService::SECRET_KEY =
    Utils::get_env_var("SECRET_KEY", "default_secret");

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

// void AuthService::verify_token(std::string& token) {
// //   auto verifier = this->create_verifier();
// //   //   auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
// //   auto decoded = this->decoded(token);
// //   verifier.verify(decoded); // will throw ???
// }


// static jwt::verifier<jwt::default_clock, jwt::traits::nlohmann_json>
// private
jwt::verifier<jwt::default_clock, jwt::traits::nlohmann_json> AuthService::create_verifier() {
  return jwt::verify<jwt::traits::nlohmann_json>()
      .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
      .with_issuer("wizzmania");
}

jwt::decoded_jwt<jwt::traits::nlohmann_json> AuthService::decode(
    const std::string& token) {
  // auto verifier = jwt::verify<jwt::traits::nlohmann_json>()
  //                 .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
  //                 .with_issuer("wizzmania");

  return jwt::decode<jwt::traits::nlohmann_json>(token);

  // auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
  // verifier.verify(decoded);
}