#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include <crow.h>

#include <optional>
#include <string>
#include <vector>

#include "auth_controller.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "user_service.hpp"
#include "websocket_manager.hpp"

class UserController {
 private:
  Database& db;
  WebSocketManager& ws;
  UserService user_service;
  AuthController auth_controller;

 public:
  explicit UserController(Database& db, WebSocketManager& ws)
      : db(db), ws(ws), user_service(db), auth_controller(ws) {}

  crow::response login(const crow::request& req);
  crow::response logout(const crow::request& req);
  crow::response send_login_error(const WizzManiaError& e);

  crow::response send_login_response(const int64_t id_user,
                                     const std::string& username,
                                     const std::string& token);
crow::response register_user(const crow::request& req); 

crow::response delete_user(int64_t id_user);
};

#endif