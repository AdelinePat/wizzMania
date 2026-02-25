#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "auth_service.hpp"
#include "exception.hpp"

// ==================== AUTH SERVICE ====================

TEST(AuthServiceTest, CreateAndValidateToken_RoundtripReturnsCorrectUserId) {
  AuthService auth;
  auto now = std::chrono::system_clock::now();
  auto expiration = now + std::chrono::hours(1);

  std::string token = auth.create_token(now, expiration, 42);
  int64_t id_user = auth.validate_token(token);

  EXPECT_EQ(id_user, 42);
}

TEST(AuthServiceTest, CreateToken_DifferentUsersProduceDifferentTokens) {
  AuthService auth;
  auto now = std::chrono::system_clock::now();
  auto exp = now + std::chrono::hours(1);

  std::string token_alice = auth.create_token(now, exp, 1);
  std::string token_bob = auth.create_token(now, exp, 2);

  EXPECT_NE(token_alice, token_bob);
}

TEST(AuthServiceTest, ValidateToken_GarbageStringThrowsUnauthorizedError) {
  AuthService auth;
  EXPECT_THROW(auth.validate_token("this.is.garbage"), UnauthorizedError);
}

TEST(AuthServiceTest, ValidateToken_EmptyStringThrowsUnauthorizedError) {
  AuthService auth;
  EXPECT_THROW(auth.validate_token(""), UnauthorizedError);
}

TEST(AuthServiceTest, ValidateToken_ExpiredTokenThrowsUnauthorizedError) {
  AuthService auth;
  auto now = std::chrono::system_clock::now();
  auto expiration = now - std::chrono::hours(1);  // already expired

  std::string token = auth.create_token(now, expiration, 99);
  EXPECT_THROW(auth.validate_token(token), UnauthorizedError);
}

TEST(AuthServiceTest, ValidateToken_TamperedTokenThrowsUnauthorizedError) {
  AuthService auth;
  auto now = std::chrono::system_clock::now();
  auto exp = now + std::chrono::hours(1);

  std::string token = auth.create_token(now, exp, 1);
  token.back() = (token.back() == 'a') ? 'b' : 'a';  // flip last char

  EXPECT_THROW(auth.validate_token(token), UnauthorizedError);
}

TEST(AuthServiceTest, CreateToken_IdUserEdgeValuesRoundtripCorrectly) {
  AuthService auth;
  auto now = std::chrono::system_clock::now();
  auto exp = now + std::chrono::hours(1);

  EXPECT_EQ(auth.validate_token(auth.create_token(now, exp, 1)), 1);
  EXPECT_EQ(auth.validate_token(auth.create_token(now, exp, 999999)), 999999);
}