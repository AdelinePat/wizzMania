#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include <gmock/gmock.h>
#include "idatabase.hpp"

// MockDatabase implements every pure virtual method from IDatabase using
// GMock's MOCK_METHOD macro. In each test you only set expectations on the
// methods that the code-under-test actually calls. All others return GMock's
// safe defaults (0, false, empty vector, nullopt, etc.) and will emit a
// warning if unexpectedly called.
class MockDatabase : public IDatabase {
 public:

  // ── Users ──────────────────────────────────────────────────────────────────

  MOCK_METHOD(int64_t, verify_user,
              (const std::string&, const std::string&),
              (override));

  MOCK_METHOD(void, user_exists,
              (int64_t),
              (override));

  MOCK_METHOD(std::optional<int64_t>, get_id_user,
              (const std::string&),
              (override));

  MOCK_METHOD(std::optional<ServerSend::Contact>, get_contact,
              (int64_t),
              (override));

  MOCK_METHOD((std::vector<ServerSend::Contact>), get_user_contacts,
              (int64_t, ChannelStatus),
              (override));

  MOCK_METHOD(bool, has_channel_access,
              (int64_t, int64_t, ChannelStatus),
              (override));

  MOCK_METHOD(bool, email_exists,
              (const std::string&),
              (override));

  MOCK_METHOD(std::optional<int64_t>, create_user,
              (const std::string&, const std::string&, const std::string&),
              (override));

  MOCK_METHOD(void, delete_user,
              (int64_t,
               (std::unordered_map<int64_t, std::unordered_set<int64_t>>&),
               (std::unordered_map<int64_t, std::unordered_set<int64_t>>&),
               ChannelStatus),
              (override));

  // ── Participants ───────────────────────────────────────────────────────────

  MOCK_METHOD((std::unordered_set<int64_t>), get_channel_participants,
              (int64_t, ChannelStatus),
              (override));

  MOCK_METHOD((std::vector<ServerSend::Contact>), get_participants,
              (int64_t, int64_t, ChannelStatus, ChannelStatus),
              (override));

  MOCK_METHOD((std::map<int64_t, std::vector<ServerSend::Contact>>),
              get_participants_and_channel,
              (int64_t, ChannelStatus, ChannelStatus),
              (override));

  MOCK_METHOD((std::vector<ServerSend::Contact>), get_channel_contacts,
              (int64_t, ChannelStatus),
              (override));

  // ── Channels ───────────────────────────────────────────────────────────────

  MOCK_METHOD((std::vector<ServerSend::ChannelInfo>), get_channels,
              (int64_t, ChannelStatus),
              (override));

  MOCK_METHOD(ServerSend::ChannelInfo, get_channel_info,
              (int64_t, int64_t, ChannelStatus),
              (override));

  MOCK_METHOD(std::optional<int64_t>, get_channel_creator,
              (int64_t),
              (override));

  MOCK_METHOD(int64_t, get_number_invited_users_in_channel,
              (int64_t, ChannelStatus, ChannelStatus),
              (override));

  MOCK_METHOD(std::optional<int64_t>, create_channel_with_participants,
              (int64_t, const std::string&, const std::string&,
               (const std::unordered_set<int64_t>&)),
              (override));

  MOCK_METHOD(void, leave_channel,
              (int64_t, int64_t),
              (override));

  MOCK_METHOD(std::optional<int64_t>, find_existing_channel,
              ((const std::unordered_set<int64_t>&)),
              (override));

  MOCK_METHOD(std::optional<bool>, does_channel_exist,
              (int64_t),
              (override));

  // ── Messages ───────────────────────────────────────────────────────────────

  MOCK_METHOD(std::optional<int64_t>, save_message,
              (int64_t, int64_t, const std::string&, const std::string&),
              (override));

  MOCK_METHOD((std::vector<ServerSend::Message>), get_channel_history,
              (int64_t, int64_t, int),
              (override));

  MOCK_METHOD(bool, update_last_read_message,
              (int64_t, int64_t, int64_t),
              (override));

  MOCK_METHOD(int64_t, get_unread_count,
              (int64_t, int64_t),
              (override));

  MOCK_METHOD((std::map<int64_t, int64_t>), get_unread_count,
              (int64_t),
              (override));

  MOCK_METHOD(ServerSend::Message, get_last_message,
              (int64_t, int64_t),
              (override));

  MOCK_METHOD((std::map<int64_t, ServerSend::Message>), get_last_messages,
              (int64_t),
              (override));

  // ── Invitations ────────────────────────────────────────────────────────────

  MOCK_METHOD((std::vector<ServerSend::ChannelInvitation>), get_invitations_base,
              (int64_t, ChannelStatus),
              (override));

  MOCK_METHOD((std::vector<ServerSend::ChannelInfo>), get_outgoing_invitations_base,
              (int64_t, ChannelStatus),
              (override));

  MOCK_METHOD(void, accept_invitation,
              (int64_t, int64_t, const std::string&),
              (override));

  MOCK_METHOD(void, reject_invitation,
              (int64_t, int64_t, const std::string&),
              (override));

  MOCK_METHOD(void, cancel_invitation,
              (int64_t, int64_t, std::string&),
              (override));
};

#endif