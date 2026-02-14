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
#include <set>
#include <string>
#include <vector>

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
  std::map<int64_t, std::set<int64_t>> get_participants_and_channel(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED);
  std::vector<ServerSend::ChannelInfo> get_channels(
      const int64_t id_user,
      ChannelStatus membership = ChannelStatus::ACCEPTED);
  std::vector<ServerSend::ChannelInfo> get_initial_channels(
      const int64_t id_user);
  ServerSend::InitialDataResponse get_initial_data(const int64_t id_user);
};

#endif