#include <crow.h>

#include <string>
#include <vector>

#include "auth.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "optional"
#include "user_service.hpp"
#include "websocket_manager.hpp"

class UserController {
 private:
  Database& db;
  //   WebSocketManager& ws_manager;
  UserService user_service;

 public:
  //   explicit UserController(Database& db, UserService& user_service)
  //       : db(db), user_service(user_service) {}

  explicit UserController(Database& db) : db(db), user_service(db) {}

  crow::response login(const crow::request& req);
  crow::response send_login_error(const HttpError& e);

  crow::response send_login_response(const int64_t id_user,
                                     const std::string& username,
                                     const std::string& token);
};