#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#include <crow.h>

#include <string>
#include <vector>

#include "auth_controller.hpp"
#include "database.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "utils.hpp"
#include "websocket_manager.hpp"
// class WebSocketManager;

class MessageHandler {
 private:
  Database& db;
  WebSocketManager& ws_manager;

  void auth_error(crow::websocket::connection& conn,
                  const std::string& message);

  void auth_success(crow::websocket::connection& conn,
                    const int64_t validated_id_user);

 public:
  explicit MessageHandler(Database& db, WebSocketManager& ws_manager)
      : db(db), ws_manager(ws_manager) {}

  // void authenticate_ws(crow::websocket::connection& conn,
  //                      const crow::json::rvalue& json_msg);

  void initial_data(crow::websocket::connection& conn);
};

#endif