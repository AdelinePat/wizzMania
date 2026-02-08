#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "auth.hpp"
#include "database.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
// using WSConn = crow::websocket::connection*;

class HttpManager {
 private:
  crow::response send_login_error(const int code, const std::string& message);
  crow::response send_login_response(const int64_t user_id, const std::string& username,  const std::string& token);

 public:
  crow::response login(Database& db, const crow::request& req);
};

#endif