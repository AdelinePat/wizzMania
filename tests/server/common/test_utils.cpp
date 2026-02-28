#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "client_exception.hpp"
// #include "doctest.h"
#include <gtest/gtest.h>

#include "client_exception.hpp"
#include "utils.hpp"

// ==================== TRIM ====================

TEST(TrimTest, RemovesSurroundingSpaces) {
  EXPECT_EQ(Utils::trim("  hello  "), "hello");
}

TEST(TrimTest, RemovesTabsAndNewlines) {
  EXPECT_EQ(Utils::trim("\t\nhello\r\n"), "hello");
}

TEST(TrimTest, NoWhitespace) { EXPECT_EQ(Utils::trim("hello"), "hello"); }

TEST(TrimTest, SingleCharacter) { EXPECT_EQ(Utils::trim("  a  "), "a"); }

TEST(TrimTest, EmptyStringThrows) { EXPECT_THROW(Utils::trim(""), BadInput); }

TEST(TrimTest, OnlyWhitespaceThrows) {
  EXPECT_THROW(Utils::trim("     "), BadInput);
}

TEST(TrimTest, OnlyTabsAndNewlinesThrows) {
  EXPECT_THROW(Utils::trim("\t\n\r"), BadInput);
}

// ==================== CLEAN USERNAME ====================

TEST(CleanUsernameTest, ValidLowercase) {
  EXPECT_EQ(Utils::clean_username("alice"), "alice");
}

TEST(CleanUsernameTest, UppercaseGetsLowercased) {
  EXPECT_EQ(Utils::clean_username("Alice"), "alice");
  EXPECT_EQ(Utils::clean_username("ALICE"), "alice");
}

TEST(CleanUsernameTest, ValidWithUnderscore) {
  EXPECT_EQ(Utils::clean_username("alice_bob"), "alice_bob");
}

TEST(CleanUsernameTest, ValidWithNumbers) {
  EXPECT_EQ(Utils::clean_username("alice123"), "alice123");
}

TEST(CleanUsernameTest, SpaceThrows) {
  EXPECT_THROW(Utils::clean_username("alice bob"), BadInput);
}

TEST(CleanUsernameTest, ExclamationMarkThrows) {
  EXPECT_THROW(Utils::clean_username("alice!"), BadInput);
}

TEST(CleanUsernameTest, AtSignThrows) {
  EXPECT_THROW(Utils::clean_username("alice@bob"), BadInput);
}

TEST(CleanUsernameTest, MixedInvalidCharsThrows) {
  EXPECT_THROW(Utils::clean_username("this!isMy_super USERNAME@"), BadInput);
}

// ==================== IS VALID PASSWORD CHARS ====================

TEST(PasswordCharsTest, NormalAscii) {
  EXPECT_TRUE(Utils::is_valid_password_chars("Password1!"));
}

TEST(PasswordCharsTest, SpecialCharsAllowed) {
  EXPECT_TRUE(Utils::is_valid_password_chars("P@ssw0rd#!"));
}

TEST(PasswordCharsTest, SpaceNotAllowed) {
  EXPECT_FALSE(Utils::is_valid_password_chars("pass word1!"));
}

TEST(PasswordCharsTest, NonAsciiNotAllowed) {
  EXPECT_FALSE(Utils::is_valid_password_chars("pässwörd1!"));
}

TEST(PasswordCharsTest, EmptyStringVacuouslyTrue) {
  // only checks character validity, length enforced by is_valid_password
  EXPECT_TRUE(Utils::is_valid_password_chars(""));
}

// ==================== IS VALID EMAIL ====================

TEST(EmailTest, ValidSimpleEmail) {
  EXPECT_TRUE(Utils::is_valid_email("alice@mail.com"));
}

TEST(EmailTest, ValidWithSubdomain) {
  EXPECT_TRUE(Utils::is_valid_email("alice@sub.domain.org"));
}

TEST(EmailTest, ValidWithDotsAndPlus) {
  EXPECT_TRUE(Utils::is_valid_email("alice.bob+tag@mail.com"));
}

TEST(EmailTest, MissingAt) {
  EXPECT_FALSE(Utils::is_valid_email("alicemail.com"));
}

TEST(EmailTest, MissingDomain) {
  EXPECT_FALSE(Utils::is_valid_email("alice@"));
}

TEST(EmailTest, MissingTLD) {
  EXPECT_FALSE(Utils::is_valid_email("alice@mail"));
}

TEST(EmailTest, EmptyString) { EXPECT_FALSE(Utils::is_valid_email("")); }

TEST(EmailTest, ContainsSpace) {
  EXPECT_FALSE(Utils::is_valid_email("alice @mail.com"));
}

// ==================== IS VALID PASSWORD ====================

TEST(PasswordTest, ValidStrongPassword) {
  EXPECT_TRUE(Utils::is_valid_password("Passw0rd!"));
}

TEST(PasswordTest, TooShort) { EXPECT_FALSE(Utils::is_valid_password("Pa1!")); }

TEST(PasswordTest, MissingUppercase) {
  EXPECT_FALSE(Utils::is_valid_password("passw0rd!"));
}

TEST(PasswordTest, MissingLowercase) {
  EXPECT_FALSE(Utils::is_valid_password("PASSW0RD!"));
}

TEST(PasswordTest, MissingDigit) {
  EXPECT_FALSE(Utils::is_valid_password("Password!"));
}

TEST(PasswordTest, MissingSpecialChar) {
  EXPECT_FALSE(Utils::is_valid_password("Passw0rd"));
}

TEST(PasswordTest, EmptyString) { EXPECT_FALSE(Utils::is_valid_password("")); }

TEST(PasswordTest, Exactly8CharsValid) {
  EXPECT_TRUE(Utils::is_valid_password("Passw0r!"));
}