#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "exception.hpp"
#include "mock_database.hpp"
#include "user_service.hpp"

using ::testing::_;
using ::testing::Return;

// ==================== USER SERVICE ====================

// The fixture creates a fresh MockDatabase and UserService before each test.
// This means no state leaks between tests — every test starts clean.
class UserServiceTest : public ::testing::Test {
 protected:
  MockDatabase mock_db;
  UserService user_service{mock_db};
};

// ── login ────────────────────────────────────────────────────────────────────

TEST_F(UserServiceTest, Login_ValidCredentials_ReturnsUserId) {
  EXPECT_CALL(mock_db, verify_user("alice", "Password1!"))
      .WillOnce(Return(42));

  int64_t result = user_service.login({"alice", "Password1!"});

  EXPECT_EQ(result, 42);
}

TEST_F(UserServiceTest, Login_InvalidCredentials_ThrowsUnauthorized) {
  EXPECT_CALL(mock_db, verify_user("alice", "wrongpassword"))
      .WillOnce(Return(-1));

  EXPECT_THROW(
      user_service.login({"alice", "wrongpassword"}),
      UnauthorizedError
  );
}

// ── has_access ───────────────────────────────────────────────────────────────

TEST_F(UserServiceTest, HasAccess_SystemUser_ReturnsTrueWithoutCallingDb) {
  // No EXPECT_CALL set — if has_channel_access is called, GMock will fail the
  // test automatically. This proves the short-circuit works.
  EXPECT_TRUE(user_service.has_access(1, 99));
}

TEST_F(UserServiceTest, HasAccess_NormalUser_DbReturnsTrueReturnsTrue) {
  EXPECT_CALL(mock_db, has_channel_access(42, 10, ChannelStatus::ACCEPTED))
      .WillOnce(Return(true));

  EXPECT_TRUE(user_service.has_access(42, 10));
}

TEST_F(UserServiceTest, HasAccess_NormalUser_DbReturnsFalseReturnsFalse) {
  EXPECT_CALL(mock_db, has_channel_access(42, 10, ChannelStatus::ACCEPTED))
      .WillOnce(Return(false));

  EXPECT_FALSE(user_service.has_access(42, 10));
}

// ── register_user ────────────────────────────────────────────────────────────

TEST_F(UserServiceTest, RegisterUser_UsernameTaken_ThrowsConflict) {
  EXPECT_CALL(mock_db, get_id_user("alice"))
      .WillOnce(Return(std::optional<int64_t>{42}));

  EXPECT_THROW(
      user_service.register_user("alice", "alice@example.com", "Password1!"),
      ConflictError
  );
}

TEST_F(UserServiceTest, RegisterUser_EmailTaken_ThrowsConflict) {
  EXPECT_CALL(mock_db, get_id_user("alice"))
      .WillOnce(Return(std::nullopt));
  EXPECT_CALL(mock_db, email_exists("alice@example.com"))
      .WillOnce(Return(true));

  EXPECT_THROW(
      user_service.register_user("alice", "alice@example.com", "Password1!"),
      ConflictError
  );
}

TEST_F(UserServiceTest, RegisterUser_DbCreateFails_ThrowsInternalError) {
  EXPECT_CALL(mock_db, get_id_user("alice"))
      .WillOnce(Return(std::nullopt));
  EXPECT_CALL(mock_db, email_exists("alice@example.com"))
      .WillOnce(Return(false));
  EXPECT_CALL(mock_db, create_user("alice", "alice@example.com", "Password1!"))
      .WillOnce(Return(std::nullopt));

  EXPECT_THROW(
      user_service.register_user("alice", "alice@example.com", "Password1!"),
      InternalError
  );
}

TEST_F(UserServiceTest, RegisterUser_HappyPath_ReturnsNewId) {
  EXPECT_CALL(mock_db, get_id_user("alice"))
      .WillOnce(Return(std::nullopt));
  EXPECT_CALL(mock_db, email_exists("alice@example.com"))
      .WillOnce(Return(false));
  EXPECT_CALL(mock_db, create_user("alice", "alice@example.com", "Password1!"))
      .WillOnce(Return(std::optional<int64_t>{7}));

  int64_t new_id =
      user_service.register_user("alice", "alice@example.com", "Password1!");

  EXPECT_EQ(new_id, 7);
}

// ── delete_user ──────────────────────────────────────────────────────────────

TEST_F(UserServiceTest, DeleteUser_SystemUser_ThrowsForbidden) {
  std::unordered_map<int64_t, std::unordered_set<int64_t>> deleted_channels;
  std::unordered_map<int64_t, std::unordered_set<int64_t>> canceled_invitations;

  EXPECT_THROW(
      user_service.delete_user(1, deleted_channels, canceled_invitations),
      ForbiddenError
  );
}