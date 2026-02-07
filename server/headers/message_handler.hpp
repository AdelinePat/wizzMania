#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H
#include <crow.h>

#include "json_helpers.hpp"
#include "websocket_manager.hpp"

// class WebSocketManager;

class MessageHandler {
 private:
  WebSocketManager& ws_manager;

 public:
  explicit MessageHandler(WebSocketManager& ws_manager)
      : ws_manager(ws_manager) {}

  void handle_send_message(crow::websocket::connection& conn, int64_t user_id,
                           const ClientSend::SendMessageRequest& req);
  void send_error(crow::websocket::connection& conn,
                  const std::string& error_code,
                  const std::string& error_message);
  // TODO: Implement
};

#endif