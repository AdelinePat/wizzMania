#ifndef MESSAGE_CONTROLLER_H
#define MESSAGE_CONTROLLER_H

#include <crow.h>

#include <optional>
#include <string>
#include <vector>

#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_service.hpp"
#include "message_structure.hpp"
#include "user_service.hpp"
#include "utils.hpp"
#include "websocket_manager.hpp"

class MessageController {
 private:
  Database& db;
  WebSocketManager& ws_manager;
  MessageService message_service;
  UserService user_service;

  void broadcast_new_message(const int64_t id_channel, const int64_t id_message,
                             const int64_t id_user, const std::string& body,
                             const std::string& timestamp,
                             const std::string& token = "");

 public:
  explicit MessageController(Database& db, WebSocketManager& ws)
      : db(db), ws_manager(ws), message_service(db), user_service(db) {}

  crow::response send_message(const crow::request& req, int64_t id_user,
                              int64_t id_channel, const std::string& token);

  void send_message_internal(int64_t id_user, int64_t id_channel,
                             std::string& body, std::string& timestamp);

  crow::response get_history(const crow::request& req, int64_t id_user,
                             int64_t id_channel);
  //   void mark_as_read(crow::websocket::connection& conn, int64_t id_user,
  //                     const crow::json::rvalue& json_msg);

  crow::response mark_as_read(const crow::request& req, int64_t id_user,
                              int64_t id_channel, const std::string& token);

  //   void wizz(crow::websocket::connection& conn, int64_t id_user,
  //             const crow::json::rvalue& json_msg);
  crow::response wizz(int64_t id_user, int64_t id_channel,
                      const std::string& token);
};

#endif