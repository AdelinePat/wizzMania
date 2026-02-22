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
#include <unordered_set>
#include <vector>

#include "exception.hpp"
#include "helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"

class Database {
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

 public:
  Database();
  Database(const std::string& host, const std::string& user,
           const std::string& password, const std::string& database);
  ~Database();
  std::mutex db_mutex;  // controllers need to access this!!

  int64_t verify_user(const std::string& username, const std::string& password);
  std::vector<ServerSend::Contact> get_user_contacts(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED);
  std::map<int64_t, ServerSend::Message> get_last_messages(
      const int64_t id_user);
  std::map<int64_t, int64_t> get_unread_count(const int64_t id_user);
  std::map<int64_t, std::vector<ServerSend::Contact>>
  get_participants_and_channel(
      const int64_t id_user, ChannelStatus membership = ChannelStatus::ACCEPTED,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED);
  std::vector<ServerSend::ChannelInfo> get_channels(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED);
  //   std::vector<ServerSend::ChannelInfo> get_initial_channels(
  //       const int64_t id_user);
  //   ServerSend::InitialDataResponse get_initial_data(const int64_t id_user);
  std::optional<int64_t> save_message(int64_t id_user, int64_t id_channel,
                                      const std::string& body,
                                      const std::string& timestamp);
  std::unordered_set<int64_t> get_channel_participants(
      int64_t id_channel, ChannelStatus membership = ChannelStatus::ACCEPTED);

  std::vector<ServerSend::Message> get_channel_history(
      int64_t id_channel, int64_t before_id_message, int limit);

  std::vector<ServerSend::ChannelInvitation> get_invitations_base(
      const int64_t id_user, ChannelStatus membership = ChannelStatus::PENDING);
  //   std::vector<ServerSend::ChannelInvitation> get_incoming_invitations(
  //       const int64_t id_user);

  bool has_channel_access(int64_t id_user, int64_t id_channel);

  void accept_invitation(int64_t id_user, int64_t id_channel,
                         const std::string& responded_at);

  void reject_invitation(int64_t id_user, int64_t id_channel,
                         const std::string& responded_at);

  //   ServerSend::ChannelInfo get_channel(
  //       int64_t id_user, int64_t id_channel,
  //       ChannelStatus membership = ChannelStatus::ACCEPTED,
  //       ChannelStatus other_membership = ChannelStatus::ACCEPTED);

  ServerSend::ChannelInfo get_channel_info(int64_t id_user, int64_t id_channel,
                                           ChannelStatus membership);

  std::vector<ServerSend::Contact> get_participants(
      const int64_t id_user, const int64_t id_channel, ChannelStatus membership,
      ChannelStatus other_membership);
  int64_t get_unread_count(const int64_t id_user, const int64_t id_channel);
  ServerSend::Message get_last_message(const int64_t id_user,
                                       const int64_t id_channel);

  //   std::vector<ServerSend::ChannelInfo> get_outgoing_invitations(
  //       int64_t id_user);
  std::vector<ServerSend::ChannelInfo> get_outgoing_invitations_base(
      int64_t id_user, ChannelStatus membership);
  std::optional<int64_t> get_channel_creator(int64_t id_channel);
  std::optional<ServerSend::Contact> get_contact(const int64_t id_user);

  std::optional<int64_t> get_id_user(const std::string& username);
  std::optional<int64_t> create_channel(const int64_t created_by,
                                        const std::string& title,
                                        const std::string& created_at);
  bool create_user_channels(const int64_t id_creator, const int64_t id_channel,
                            const std::unordered_set<int64_t>& participants);
  std::optional<int64_t> create_channel_with_participants(
      const int64_t id_creator, const std::string& title,
      const std::string& created_at,
      const std::unordered_set<int64_t>& participants);
  std::vector<ServerSend::Contact> get_channel_contacts(
      int64_t id_channel, ChannelStatus membership);

  int64_t get_number_invited_users_in_channel(
    int64_t id_channel,
    ChannelStatus membership = ChannelStatus::PENDING,
    ChannelStatus other_membership = ChannelStatus::ACCEPTED);
};
#endif