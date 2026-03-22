#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "helpers.hpp"

// ==================== PASSWORD HELPER ====================
//
// PasswordHelper wraps bcrypt. These tests verify:
//   - hash_password produces a valid bcrypt string (60 chars, $2 prefix)
//   - verify_password returns true for the correct password
//   - verify_password returns false for a wrong password
//   - two hashes of the same password differ (different salts each time)
//   - an empty password can be hashed and verified like any other input
//
// No mock needed — PasswordHelper is pure computation, no dependencies.

// ── hash_password
// ─────────────────────────────────────────────────────────────

TEST(PasswordHelperTest, HashPassword_ReturnsBcryptString) {
  std::string hash = PasswordHelper::hash_password("Password1!");

  // bcrypt output is always exactly 60 characters
  EXPECT_EQ(hash.size(), 60u);
  // and always starts with the bcrypt version prefix
  EXPECT_EQ(hash.substr(0, 4), "$2a$");
}

TEST(PasswordHelperTest, HashPassword_TwoCalls_ProduceDifferentHashes) {
  // bcrypt generates a fresh random salt on every call —
  // same password must never produce the same hash twice
  std::string hash1 = PasswordHelper::hash_password("Password1!");
  std::string hash2 = PasswordHelper::hash_password("Password1!");

  EXPECT_NE(hash1, hash2);
}

// ── verify_password
// ───────────────────────────────────────────────────────────

TEST(PasswordHelperTest, VerifyPassword_CorrectPassword_ReturnsTrue) {
  std::string hash = PasswordHelper::hash_password("Password1!");

  EXPECT_TRUE(PasswordHelper::verify_password("Password1!", hash));
}

TEST(PasswordHelperTest, VerifyPassword_WrongPassword_ReturnsFalse) {
  std::string hash = PasswordHelper::hash_password("Password1!");

  EXPECT_FALSE(PasswordHelper::verify_password("WrongPassword1!", hash));
}

TEST(PasswordHelperTest, VerifyPassword_EmptyPassword_ReturnsFalse) {
  std::string hash = PasswordHelper::hash_password("Password1!");

  EXPECT_FALSE(PasswordHelper::verify_password("", hash));
}

TEST(PasswordHelperTest,
     VerifyPassword_EmptyPasswordHashedThenVerified_ReturnsTrue) {
  // Edge case: empty string is a valid bcrypt input — hash and verify should
  // be consistent even if the service rejects empty passwords upstream.
  std::string hash = PasswordHelper::hash_password("");

  EXPECT_TRUE(PasswordHelper::verify_password("", hash));
}

TEST(PasswordHelperTest, VerifyPassword_DifferentHashSamePassword_ReturnsTrue) {
  // Two different hashes of the same password must both verify correctly —
  // this confirms bcrypt extracts the embedded salt properly each time.
  std::string hash1 = PasswordHelper::hash_password("Password1!");
  std::string hash2 = PasswordHelper::hash_password("Password1!");

  EXPECT_TRUE(PasswordHelper::verify_password("Password1!", hash1));
  EXPECT_TRUE(PasswordHelper::verify_password("Password1!", hash2));
}