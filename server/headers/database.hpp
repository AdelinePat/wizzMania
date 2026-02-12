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
#include <memory>
#include <mutex>
#include <optional>
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
  std::vector<ServerSend::Contact> get_contact(const int64_t id_user);
  // TODO: Implement
};

#endif