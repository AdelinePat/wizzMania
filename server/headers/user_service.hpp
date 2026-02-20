#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "auth.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
// using WSConn = crow::websocket::connection*;

class UserService {
  Database& db;

 private:
  // crow::response send_login_error(const int code, const std::string& message);
  // crow::response send_login_response(const int64_t id_user,
  //                                    const std::string& username,
  //                                    const std::string& token);

 public:
  explicit UserService(Database& db) : db(db) {}

  // crow::response login(Database& db, const crow::request& req);
  int64_t login(AuthMessages::LoginRequest login_request);
};

#endif