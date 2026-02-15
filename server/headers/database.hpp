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

#include "helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"

class Database {
 private:
  sql::Driver* driver;
  std::unique_ptr<sql::Connection> conn;
  std::mutex db_mutex;

  std::string host;
  std::string user;
  std::string password;
  std::string database;

  void start_connection(const std::string& host, const std::string& user,
                        const std::string& password,
                        const std::string& database);
  void ensure_connection();

  bool update_invitation(int64_t id_user, int64_t id_channel,
                         ChannelStatus membership);

 public:
  Database();
  Database(const std::string& host, const std::string& user,
           const std::string& password, const std::string& database);
  ~Database();

  int64_t verify_user(const std::string& username, const std::string& password);
  std::vector<ServerSend::Contact> get_contact(
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
  std::vector<ServerSend::ChannelInfo> get_initial_channels(
      const int64_t id_user);
  ServerSend::InitialDataResponse get_initial_data(const int64_t id_user);
  std::optional<int64_t> save_message(int64_t id_user, int64_t id_channel,
                                      const std::string& body,
                                      const std::string& timestamp);
  std::unordered_set<int64_t> get_channel_participants(
      int64_t id_channel, ChannelStatus membership = ChannelStatus::ACCEPTED);

  std::vector<ServerSend::Message> get_channel_history(
      int64_t id_channel, int64_t before_id_message, int limit);

  std::vector<ServerSend::ChannelInvitation> get_invitations_base(
      const int64_t id_user, ChannelStatus membership = ChannelStatus::PENDING);
  std::vector<ServerSend::ChannelInvitation> get_initial_invitations(
      const int64_t id_user);

  bool has_channel_access(int64_t id_user, int64_t id_channel);

  bool accept_invitation(int64_t id_user, int64_t id_channel);

  bool reject_invitation(int64_t id_user, int64_t id_channel);

  ServerSend::ChannelInfo get_channel(
      int64_t id_user, int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED);

  ServerSend::ChannelInfo get_channel_info(int64_t id_user, int64_t id_channel,
                                           ChannelStatus membership);

  std::vector<ServerSend::Contact> get_participants(
      const int64_t id_user, const int64_t id_channel, ChannelStatus membership,
      ChannelStatus other_membership);
  int64_t get_unread_count(const int64_t id_user, const int64_t id_channel);
  ServerSend::Message get_last_message(const int64_t id_user,
                                       const int64_t id_channel);
};

#endif