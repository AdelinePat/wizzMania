#include "auth.hpp"

std::string Auth::generateToken(int64_t user_id) {
    auto now = std::chrono::system_clock::now();
    auto expiration = now + std::chrono::hours(24 * 7);  // 7 days
    
    // Use basic_claim with nlohmann_json traits
    using claim = jwt::basic_claim<jwt::traits::nlohmann_json>;
    
    return jwt::create<jwt::traits::nlohmann_json>()
        .set_type("JWT")
        .set_issuer("wizzmania")
        .set_issued_at(now)
        .set_expires_at(expiration)
        .set_payload_claim("user_id", claim(std::to_string(user_id)))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
}

std::optional<int64_t> Auth::validateToken(const std::string& token) {
    try {
        auto verifier = jwt::verify<jwt::traits::nlohmann_json>()
            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
            .with_issuer("wizzmania");
        
        auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
        verifier.verify(decoded);
        
        std::string user_id_str = decoded.get_payload_claim("user_id").as_string();
        return std::stoll(user_id_str);
        
    } catch (const std::exception&) {
        return std::nullopt;
    }
}