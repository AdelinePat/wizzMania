#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "client_exception.hpp"
#include "doctest.h"
#include "utils.hpp"

// ==================== TRIM ====================

TEST_CASE("trim - normal string with surrounding spaces") {
  CHECK(Utils::trim("  hello  ") == "hello");
}

TEST_CASE("trim - tabs and newlines") {
  CHECK(Utils::trim("\t\nhello\r\n") == "hello");
}

TEST_CASE("trim - no whitespace") { CHECK(Utils::trim("hello") == "hello"); }

TEST_CASE("trim - single character") { CHECK(Utils::trim("  a  ") == "a"); }

TEST_CASE("trim - empty string throws") {
  CHECK_THROWS_AS(Utils::trim(""), BadInput);
}

TEST_CASE("trim - only whitespace throws") {
  CHECK_THROWS_AS(Utils::trim("     "), BadInput);
}

TEST_CASE("trim - only tabs and newlines throws") {
  CHECK_THROWS_AS(Utils::trim("\t\n\r"), BadInput);
}

// ==================== CLEAN USERNAME ====================

TEST_CASE("clean_username - valid lowercase") {
  CHECK(Utils::clean_username("alice") == "alice");
}

TEST_CASE("clean_username - valid uppercase gets lowercased") {
  CHECK(Utils::clean_username("Alice") == "alice");
  CHECK(Utils::clean_username("ALICE") == "alice");
}

TEST_CASE("clean_username - valid with underscore") {
  CHECK(Utils::clean_username("alice_bob") == "alice_bob");
}

TEST_CASE("clean_username - valid with numbers") {
  CHECK(Utils::clean_username("alice123") == "alice123");
}

TEST_CASE("clean_username - invalid space throws") {
  CHECK_THROWS_AS(Utils::clean_username("alice bob"), BadInput);
}

TEST_CASE("clean_username - invalid exclamation mark throws") {
  CHECK_THROWS_AS(Utils::clean_username("alice!"), BadInput);
}

TEST_CASE("clean_username - invalid at sign throws") {
  CHECK_THROWS_AS(Utils::clean_username("alice@bob"), BadInput);
}

TEST_CASE("clean_username - invalid mixed throws") {
  CHECK_THROWS_AS(Utils::clean_username("this!isMy_super USERNAME@"),
                  BadInput);
}

// ==================== IS VALID PASSWORD CHARS ====================

TEST_CASE("is_valid_password_chars - normal ascii") {
  CHECK(Utils::is_valid_password_chars("Password1!") == true);
}

TEST_CASE("is_valid_password_chars - special chars allowed") {
  CHECK(Utils::is_valid_password_chars("P@ssw0rd#!") == true);
}

TEST_CASE("is_valid_password_chars - space not allowed") {
  CHECK(Utils::is_valid_password_chars("pass word1!") == false);
}

TEST_CASE("is_valid_password_chars - non ASCII not allowed") {
  CHECK(Utils::is_valid_password_chars("pässwörd1!") == false);
}

TEST_CASE("is_valid_password_chars - empty string") {
  CHECK(Utils::is_valid_password_chars("") ==
        true);  // vacuously true, caught by other validators
}

TEST_CASE("is_valid_password - empty string is invalid") {
    CHECK(Utils::is_valid_password("") == false); // caught by size() < 8
}

// ==================== IS VALID EMAIL ====================

TEST_CASE("is_valid_email - valid simple email") {
  CHECK(Utils::is_valid_email("alice@mail.com") == true);
}

TEST_CASE("is_valid_email - valid with subdomain") {
  CHECK(Utils::is_valid_email("alice@sub.domain.org") == true);
}

TEST_CASE("is_valid_email - valid with dots and plus") {
  CHECK(Utils::is_valid_email("alice.bob+tag@mail.com") == true);
}

TEST_CASE("is_valid_email - missing @") {
  CHECK(Utils::is_valid_email("alicemail.com") == false);
}

TEST_CASE("is_valid_email - missing domain") {
  CHECK(Utils::is_valid_email("alice@") == false);
}

TEST_CASE("is_valid_email - missing TLD") {
  CHECK(Utils::is_valid_email("alice@mail") == false);
}

TEST_CASE("is_valid_email - empty string") {
  CHECK(Utils::is_valid_email("") == false);
}

TEST_CASE("is_valid_email - spaces") {
  CHECK(Utils::is_valid_email("alice @mail.com") == false);
}

// ==================== IS VALID PASSWORD ====================

TEST_CASE("is_valid_password - valid strong password") {
  CHECK(Utils::is_valid_password("Passw0rd!") == true);
}

TEST_CASE("is_valid_password - too short") {
  CHECK(Utils::is_valid_password("Pa1!") == false);
}

TEST_CASE("is_valid_password - missing uppercase") {
  CHECK(Utils::is_valid_password("passw0rd!") == false);
}

TEST_CASE("is_valid_password - missing lowercase") {
  CHECK(Utils::is_valid_password("PASSW0RD!") == false);
}

TEST_CASE("is_valid_password - missing digit") {
  CHECK(Utils::is_valid_password("Password!") == false);
}

TEST_CASE("is_valid_password - missing special char") {
  CHECK(Utils::is_valid_password("Passw0rd") == false);
}

TEST_CASE("is_valid_password - empty string") {
  CHECK(Utils::is_valid_password("") == false);
}

TEST_CASE("is_valid_password - exactly 8 chars valid") {
  CHECK(Utils::is_valid_password("Passw0r!") == true);
}