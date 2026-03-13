#ifndef DATABASE_H
#define DATABASE_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "exception.hpp"
#include "helpers.hpp"
#include "idatabase.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "utils.hpp"

class Database : public IDatabase {
 private:
  sql::Driver* driver;
  std::unique_ptr<sql::Connection> conn;

  std::string host;
  std::string user;
  std::string password;
  std::string database;

  void start_connection(const std::string& host, const std::string& user,
                        const std::string& password,
                        const std::string& database);
  void ensure_connection();

  void update_invitation(int64_t id_user, int64_t id_channel,
                         const std::string& responded_at,
                         ChannelStatus membership);
  int leave_channel(int64_t id_user, int64_t id_channel,
                    ChannelStatus membership);

  std::optional<int64_t> create_channel(const int64_t created_by,
                                        const std::string& title,
                                        const std::string& created_at);

  bool create_user_channels(const int64_t id_creator, const int64_t id_channel,
                            const std::unordered_set<int64_t>& participants);

 public:
  Database();
  Database(const std::string& host, const std::string& user,
           const std::string& password, const std::string& database);
  ~Database();
  //   std::mutex db_mutex;  // controllers need to access this!!
  std::recursive_mutex db_mutex;

  // ── Users ──────────────────────────────────────────────────────────────────

  int64_t verify_user(const std::string& identifier,
                      const std::string& password) override;
  void user_exists(const int64_t id_user) override;
  std::optional<int64_t> get_id_user(const std::string& username) override;
  std::optional<ServerSend::Contact> get_contact(
      const int64_t id_user) override;
  std::vector<ServerSend::Contact> get_user_contacts(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED) override;
  bool has_channel_access(
      int64_t id_user, int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED) override;
  bool email_exists(const std::string& email) override;
  std::optional<int64_t> create_user(const std::string& username,
                                     const std::string& email,
                                     const std::string& password) override;
  void delete_user(int64_t id_user,
                   std::unordered_map<int64_t, std::unordered_set<int64_t>>&
                       deleted_channels,
                   std::unordered_map<int64_t, std::unordered_set<int64_t>>&
                       canceled_invitations,
                   ChannelStatus membership = ChannelStatus::ACCEPTED)
      override;  // true if user is delete

  // ── Participants (database_user.cpp) ──────────────────────────────────────

  std::unordered_set<int64_t> get_channel_participants(
      int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED) override;
  std::vector<ServerSend::Contact> get_participants(
      const int64_t id_user, const int64_t id_channel, ChannelStatus membership,
      ChannelStatus other_membership) override;
  std::map<int64_t, std::vector<ServerSend::Contact>>
  get_participants_and_channel(
      const int64_t id_user, ChannelStatus membership = ChannelStatus::ACCEPTED,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED) override;
  std::vector<ServerSend::Contact> get_channel_contacts(
      int64_t id_channel, ChannelStatus membership) override;

  // ── Channels ───────────────────────────────────────────────────────────────

  std::vector<ServerSend::ChannelInfo> get_channels(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED) override;
  ServerSend::ChannelInfo get_channel_info(int64_t id_user, int64_t id_channel,
                                           ChannelStatus membership) override;
  std::optional<int64_t> get_channel_creator(int64_t id_channel) override;
  int64_t get_number_invited_users_in_channel(
      int64_t id_channel, ChannelStatus membership = ChannelStatus::PENDING,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED) override;
  std::optional<int64_t> create_channel_with_participants(
      const int64_t id_creator, const std::string& title,
      const std::string& created_at,
      const std::unordered_set<int64_t>& participants) override;
  void leave_channel(int64_t id_user, int64_t id_channel) override;
  std::optional<int64_t> find_existing_channel(
      const std::unordered_set<int64_t>& all_participants) override;
  std::optional<bool> does_channel_exist(int64_t id_channel) override;

  // ── Invitations ────────────────────────────────────────────────────────────

  std::vector<ServerSend::ChannelInvitation> get_invitations_base(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::PENDING) override;
  std::vector<ServerSend::ChannelInfo> get_outgoing_invitations_base(
      int64_t id_user, ChannelStatus membership) override;
  void accept_invitation(int64_t id_user, int64_t id_channel,
                         const std::string& responded_at) override;

  void reject_invitation(int64_t id_user, int64_t id_channel,
                         const std::string& responded_at) override;

  void cancel_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at) override;

  // ── Messages ───────────────────────────────────────────────────────────────
  std::optional<int64_t> save_message(int64_t id_user, int64_t id_channel,
                                      const std::string& body,
                                      const std::string& timestamp) override;
  std::vector<ServerSend::Message> get_channel_history(
      int64_t id_channel, int64_t before_id_message, int limit) override;
  bool update_last_read_message(int64_t id_user, int64_t id_channel,
                                int64_t last_read_id_message) override;
  int64_t get_unread_count(const int64_t id_user,
                           const int64_t id_channel) override;
  std::map<int64_t, int64_t> get_unread_count(const int64_t id_user) override;
  ServerSend::Message get_last_message(const int64_t id_user,
                                       const int64_t id_channel) override;

  std::map<int64_t, ServerSend::Message> get_last_messages(
      const int64_t id_user) override;
};

#endif