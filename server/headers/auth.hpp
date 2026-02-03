#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <optional>
#include <cstdint>
#include <chrono>

#define JWT_DISABLE_PICOJSON
#include <nlohmann/json.hpp>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>  

class Auth {
public:
    static std::string generateToken(int64_t user_id);
    static std::optional<int64_t> validateToken(const std::string& token);
    
private:
    static constexpr const char* SECRET_KEY = "your_secret_key_change_in_production";
};

#endif