#ifndef MESSAGE_CONTROLLER_H
#define MESSAGE_CONTROLLER_H

#include <crow.h>

#include <string>
#include <vector>

#include "database.hpp"      // use
#include "exception.hpp"     // use
#include "helpers.hpp"       // use
#include "json_helpers.hpp"  // use
#include "message_service.hpp"
#include "message_structure.hpp"  // use
#include "optional"               // use
#include "user_service.hpp"
#include "websocket_manager.hpp"
#include "utils.hpp"

class MessageController {
 private:
  Database& db;
  WebSocketManager& ws_manager;
  MessageService message_service;
  UserService user_service;

  void broadcast_new_message(const int64_t id_channel, const int64_t id_message,
                             const int64_t id_user, const std::string& body,
                             const std::string& timestamp);

 public:
  explicit MessageController(Database& db, WebSocketManager& ws)
      : db(db), ws_manager(ws), message_service(db), user_service(db) {}

  void send_message(crow::websocket::connection& conn, int64_t id_user,
                    const crow::json::rvalue& json_msg);

  void send_message_internal(int64_t id_user, int64_t id_channel,
                             std::string& body, std::string& timestamp);

  void send_history(crow::websocket::connection& conn, int64_t id_user,
                    const crow::json::rvalue& json_msg);

  void send_history_response(crow::websocket::connection& conn,
                             int64_t id_channel,
                             std::vector<ServerSend::Message>& messages,
                             int limit);
};

#endif