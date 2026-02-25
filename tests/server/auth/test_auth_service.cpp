#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "auth_service.hpp"
#include "exception.hpp"
#include <chrono>
#include <thread>

// ==================== AUTH SERVICE ====================

TEST_CASE("create_token + validate_token - roundtrip returns correct id_user") {
    AuthService auth;
    auto now = std::chrono::system_clock::now();
    auto expiration = now + std::chrono::hours(1);

    std::string token = auth.create_token(now, expiration, 42);
    int64_t id_user = auth.validate_token(token);

    CHECK(id_user == 42);
}

TEST_CASE("create_token - different users produce different tokens") {
    AuthService auth;
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(1);

    std::string token_alice = auth.create_token(now, exp, 1);
    std::string token_bob   = auth.create_token(now, exp, 2);

    CHECK(token_alice != token_bob);
}

TEST_CASE("validate_token - garbage string throws UnauthorizedError") {
    AuthService auth;
    CHECK_THROWS_AS(auth.validate_token("this.is.garbage"), UnauthorizedError);
}

TEST_CASE("validate_token - empty string throws UnauthorizedError") {
    AuthService auth;
    CHECK_THROWS_AS(auth.validate_token(""), UnauthorizedError);
}

TEST_CASE("validate_token - expired token throws UnauthorizedError") {
    AuthService auth;
    auto now = std::chrono::system_clock::now();
    auto expiration = now - std::chrono::hours(1); // already expired

    std::string token = auth.create_token(now, expiration, 99);
    CHECK_THROWS_AS(auth.validate_token(token), UnauthorizedError);
}

TEST_CASE("validate_token - tampered token throws UnauthorizedError") {
    AuthService auth;
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(1);

    std::string token = auth.create_token(now, exp, 1);
    token.back() = (token.back() == 'a') ? 'b' : 'a'; // flip last char

    CHECK_THROWS_AS(auth.validate_token(token), UnauthorizedError);
}

TEST_CASE("create_token - id_user 1 and large id roundtrip correctly") {
    AuthService auth;
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::hours(1);

    CHECK(auth.validate_token(auth.create_token(now, exp, 1))       == 1);
    CHECK(auth.validate_token(auth.create_token(now, exp, 999999))  == 999999);
}