#include "auth_controller.hpp"



std::string AuthController::generateToken(int64_t id_user) {
  auto now = std::chrono::system_clock::now();
  auto expiration = now + std::chrono::hours(24 * 7);  // 7 days

  
  
  return AuthService::create_token(now, expiration, id_user);

  //   return jwt::create<jwt::traits::nlohmann_json>()
  //       .set_type("JWT")
  //       .set_issuer("wizzmania")
  //       .set_issued_at(now)
  //       .set_expires_at(expiration)
  //       .set_payload_claim("id_user", claim(std::to_string(id_user)))
  //       .sign(jwt::algorithm::hs256{SECRET_KEY});
}

std::optional<int64_t> AuthController::validateToken(const std::string& token) {
  try {
    // auto verifier = jwt::verify<jwt::traits::nlohmann_json>()
    //                     .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
    //                     .with_issuer("wizzmania");

    // auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
    auto verifier = AuthService::create_verifier();
    auto decoded = AuthService::decode(token);
    verifier.verify(decoded);

    std::string id_user_str = decoded.get_payload_claim("id_user").as_string();
    return std::stoll(id_user_str);

  } catch (const std::exception&) {
    return std::nullopt;
  }
}