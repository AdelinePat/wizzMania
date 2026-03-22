#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "exception.hpp"
#include "message_controller.hpp"
#include "mock_database.hpp"
#include "mock_websocket_manager.hpp"

using ::testing::_;
using ::testing::Return;

// ==================== MESSAGE CONTROLLER ====================

class MessageControllerTest : public ::testing::Test {
 protected:
  MockDatabase mock_db;
  MockWebSocketManager mock_ws;
  MessageController message_controller{mock_db, mock_ws};
};

// ── send_message_internal ────────────────────────────────────────────────────
// This is the server-initiated send (system user messages, join notifications).
// Token is always empty → broadcast_new_message sends to ALL participants
// via broadcast_to_users only. send_to_user_except must NOT be called.

TEST_F(MessageControllerTest,
       SendMessageInternal_NoToken_BroadcastsToAllParticipants) {
  const int64_t id_user = 1;     // system user
  const int64_t id_channel = 10;
  std::string body = "Welcome!";
  std::string timestamp = "2026-01-01T00:00:00Z";

  // system user always has access (short-circuit in has_access)
  // save_message returns a new id
  EXPECT_CALL(mock_db, save_message(id_user, id_channel, body, timestamp))
      .WillOnce(Return(std::optional<int64_t>{99}));

  // participants fetched to know who to broadcast to
  std::unordered_set<int64_t> participants = {1, 2, 3};
  EXPECT_CALL(mock_db, get_channel_participants(id_channel, ChannelStatus::ACCEPTED))
      .WillOnce(Return(participants));

  // empty token → broadcast_to_users called once for everyone
  EXPECT_CALL(mock_ws, broadcast_to_users(participants, _)).Times(1);

  // send_to_user_except must NOT be called — no token means no exclusion
  EXPECT_CALL(mock_ws, send_to_user_except(_, _, _)).Times(0);

  message_controller.send_message_internal(id_user, id_channel, body, timestamp);
}

// ── wizz ─────────────────────────────────────────────────────────────────────
// wizz always has a token (HTTP request from a real user).
// Pattern: send_to_user_except for sender's other sessions,
//          then broadcast_to_users for all OTHER participants (sender erased).

TEST_F(MessageControllerTest,
       Wizz_WithToken_ExcludesSenderAndBroadcastsToRest) {
  const int64_t id_user = 42;
  const int64_t id_channel = 10;
  const std::string token = "user42-token";

  EXPECT_CALL(mock_db, has_channel_access(id_user, id_channel, ChannelStatus::ACCEPTED))
      .WillOnce(Return(true));

  std::unordered_set<int64_t> participants = {42, 7, 8};
  EXPECT_CALL(mock_db, get_channel_participants(id_channel, ChannelStatus::ACCEPTED))
      .WillOnce(Return(participants));

  // sender's OTHER sessions excluded by token
  EXPECT_CALL(mock_ws, send_to_user_except(id_user, _, token)).Times(1);

  // everyone except sender gets the broadcast
  std::unordered_set<int64_t> others = {7, 8};
  EXPECT_CALL(mock_ws, broadcast_to_users(others, _)).Times(1);

  message_controller.wizz(id_user, id_channel, token);
}

TEST_F(MessageControllerTest, Wizz_UserHasNoAccess_ThrowsForbidden) {
  EXPECT_CALL(mock_db, has_channel_access(42, 10, ChannelStatus::ACCEPTED))
      .WillOnce(Return(false));

  EXPECT_THROW(
      message_controller.wizz(42, 10, "some-token"),
      ForbiddenError
  );
}