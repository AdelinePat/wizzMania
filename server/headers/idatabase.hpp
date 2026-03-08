#ifndef IDATABASE_H
#define IDATABASE_H

#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "message_structure.hpp"
#include "messages.hpp"

// Pure interface for Database — allows mocking in tests.
// Database inherits from this. Services take IDatabase& instead of Database&.
class IDatabase {
 public:
  virtual ~IDatabase() = default;

  // ── Users ──────────────────────────────────────────────────────────────────

  virtual int64_t verify_user(const std::string& username,
                              const std::string& password) = 0;

  virtual void user_exists(const int64_t id_user) = 0;

  virtual std::optional<int64_t> get_id_user(const std::string& username) = 0;

  virtual std::optional<ServerSend::Contact> get_contact(
      const int64_t id_user) = 0;

  virtual std::vector<ServerSend::Contact> get_user_contacts(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED) = 0;

  virtual bool has_channel_access(
      int64_t id_user, int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED) = 0;

  virtual bool email_exists(const std::string& email) = 0;

  virtual std::optional<int64_t> create_user(const std::string& username,
                                             const std::string& email,
                                             const std::string& password) = 0;
  virtual void delete_user(
      int64_t id_user,
      std::unordered_map<int64_t, std::unordered_set<int64_t>>&
          deleted_channels,
      std::unordered_map<int64_t, std::unordered_set<int64_t>>&
          canceled_invitations,
      ChannelStatus membership =
          ChannelStatus::ACCEPTED) = 0;  // true if user is delete

  // ── Participants (database_user.cpp) ──────────────────────────────────────

  virtual std::unordered_set<int64_t> get_channel_participants(
      int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED) = 0;

  virtual std::vector<ServerSend::Contact> get_participants(
      const int64_t id_user, const int64_t id_channel, ChannelStatus membership,
      ChannelStatus other_membership) = 0;

  virtual std::map<int64_t, std::vector<ServerSend::Contact>>
  get_participants_and_channel(
      const int64_t id_user, ChannelStatus membership = ChannelStatus::ACCEPTED,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED) = 0;

  virtual std::vector<ServerSend::Contact> get_channel_contacts(
      int64_t id_channel, ChannelStatus membership) = 0;

  // ── Channels ───────────────────────────────────────────────────────────────

  virtual std::vector<ServerSend::ChannelInfo> get_channels(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED) = 0;

  virtual ServerSend::ChannelInfo get_channel_info(
      int64_t id_user, int64_t id_channel, ChannelStatus membership) = 0;

  virtual std::optional<int64_t> get_channel_creator(int64_t id_channel) = 0;

  virtual int64_t get_number_invited_users_in_channel(
      int64_t id_channel, ChannelStatus membership = ChannelStatus::PENDING,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED) = 0;

  virtual std::optional<int64_t> create_channel_with_participants(
      const int64_t id_creator, const std::string& title,
      const std::string& created_at,
      const std::unordered_set<int64_t>& participants) = 0;

  virtual void leave_channel(int64_t id_user, int64_t id_channel) = 0;

  virtual std::optional<int64_t> find_existing_channel(
      const std::unordered_set<int64_t>& all_participants) = 0;

  virtual std::optional<bool> does_channel_exist(int64_t id_channel) = 0;

  // ── Invitations ────────────────────────────────────────────────────────────

  virtual std::vector<ServerSend::ChannelInvitation> get_invitations_base(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::PENDING) = 0;

  virtual std::vector<ServerSend::ChannelInfo> get_outgoing_invitations_base(
      int64_t id_user, ChannelStatus membership) = 0;

  virtual void accept_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) = 0;

  virtual void reject_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) = 0;

  virtual void cancel_invitation(int64_t id_user, int64_t id_channel,
                                 std::string& responded_at) = 0;

  

  // ── Messages ───────────────────────────────────────────────────────────────

  virtual std::optional<int64_t> save_message(int64_t id_user,
                                              int64_t id_channel,
                                              const std::string& body,
                                              const std::string& timestamp) = 0;

  virtual std::vector<ServerSend::Message> get_channel_history(
      int64_t id_channel, int64_t before_id_message, int limit) = 0;

  virtual bool update_last_read_message(int64_t id_user, int64_t id_channel,
                                        int64_t last_read_id_message) = 0;

  virtual int64_t get_unread_count(const int64_t id_user,
                                   const int64_t id_channel) = 0;

  virtual std::map<int64_t, int64_t> get_unread_count(
      const int64_t id_user) = 0;

  virtual ServerSend::Message get_last_message(const int64_t id_user,
                                               const int64_t id_channel) = 0;
  virtual std::map<int64_t, ServerSend::Message> get_last_messages(
      const int64_t id_user) = 0;
};

#endif