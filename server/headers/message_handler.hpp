#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#include <crow.h>

#include <string>
#include <vector>

#include "auth.hpp"
#include "database.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "websocket_manager.hpp"
// class WebSocketManager;

class MessageHandler {
 private:
  Database& db;
  WebSocketManager& ws_manager;
  void send_error(crow::websocket::connection& conn,
                  const std::string& error_code,
                  const std::string& error_message);

  void auth_error(crow::websocket::connection& conn,
                  const std::string& message);

  void auth_success(crow::websocket::connection& conn,
                    const int64_t validated_id_user);

 public:
  explicit MessageHandler(Database& db, WebSocketManager& ws_manager)
      : db(db), ws_manager(ws_manager) {}

  void send_message(crow::websocket::connection& conn, int64_t id_user,
                    const crow::json::rvalue& json_msg);

  void authenticate_ws(crow::websocket::connection& conn,
                       const crow::json::rvalue& json_msg);

  void initial_data(crow::websocket::connection& conn);

  void create_channel(crow::websocket::connection& conn, int64_t id_user,
                      const crow::json::rvalue& json_msg);

  void send_history(crow::websocket::connection& conn, int64_t id_user,
                    const crow::json::rvalue& json_msg);
  void accept_invitation(crow::websocket::connection& conn, int64_t id_user,
                         const crow::json::rvalue& json_msg);
  void reject_invitation(crow::websocket::connection& conn, int64_t id_user,
                         const crow::json::rvalue& json_msg);
  void broadcast_new_message(const int64_t id_channel, const int64_t id_message,
                             const int64_t id_user, const std::string& body,
                             const std::string& timestamp);

  void broadcast_joined_notification(
      const int64_t id_user, const int64_t id_channel,
      std::vector<ServerSend::Contact>& participants);

  // TODO: Implement
};

#endif