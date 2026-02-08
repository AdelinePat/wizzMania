#include "message_handler.hpp"

void MessageHandler::handle_send_message(crow::websocket::connection& conn,
                                         int64_t user_id,
                                         const crow::json::rvalue& json_msg) {
  //    Shouldn't this part be inside this function???

  std::optional<::ClientSend::SendMessageRequest> req =
      JsonHelpers::ClientSend::parse_send_message(json_msg);
  if (!req.has_value()) {
    this->send_error(conn, "INVALID_FORMAT", "Invalid SEND_MESSAGE format");
    return;
  }

  std::cout << "[MSG] User " << user_id << " -> Channel " << req->channel_id
            << ": " << req->body << "\n";

  // TODO: Validate channel access
  // TODO: Store message in database

  // TEMP : ECHO MSG BACK
  ServerSend::NewMessageBroadcast broadcast;             // outer struct
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;  // ✅ this exists here
  broadcast.channel_id = req->channel_id;

  // fill the inner message
  broadcast.message.message_id = 0;  // fake id for now TODO get from DB!!
  broadcast.message.sender_id = user_id;
//   broadcast.message.sender_username = "whatever";
  broadcast.message.body = req->body;

  // timestamp as string
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&now_time), "%Y-%m-%dT%H:%M:%S");
  broadcast.message.timestamp = oss.str();

  broadcast.message.is_system = true;

  std::string json_str = JsonHelpers::ServerSend::to_json(broadcast).dump();
  ws_manager.broadcast_to_all(json_str);

  // send JSON
  //   conn.send_text(JsonHelpers::ServerSend::to_json(broadcast).dump());
}

void MessageHandler::send_error(crow::websocket::connection& conn,
                                const std::string& error_code,
                                const std::string& error_message) {
  ServerSend::ErrorResponse err;
  err.type = WizzMania::MessageType::ERROR;
  err.error_code = error_code;
  err.message = error_message;

  conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
}