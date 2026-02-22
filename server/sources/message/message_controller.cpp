#include "message_controller.hpp"

void MessageController::send_message(crow::websocket::connection& conn,
                                     int64_t id_user,
                                     const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::SendMessageRequest> req =
      JsonHelpers::ClientSend::parse_send_message(json_msg);
  try {
    if (!req.has_value()) {
      throw BadRequestError("Invalid SEND_MESSAGE format");
    }

    std::cout << "[MSG] User " << id_user << " -> Channel " << req->id_channel
              << ": " << req->body << "\n";

    int64_t id_channel = req->id_channel;
    bool has_access = user_service.has_access(id_user, id_channel);
    if (!has_access) {
      throw BadRequestError(
          "User has no permission to SEND_MESSAGE to this channel");
    }

    std::string body = req->body;
    std::string timestamp = Utils::get_timestamp();

    int64_t new_id_message =
        message_service.create_message(id_user, id_channel, body, timestamp);
    this->broadcast_new_message(id_channel, new_id_message, id_user, body,
                                timestamp);
  } catch (const WizzManiaError& e) {
    // return send_error(conn, "INTERNAL_ERROR", e.get_message());
    WizzManiaError::send_ws_error(conn, e);
  }
}

void MessageController::send_message_internal(int64_t id_user,
                                              int64_t id_channel,
                                              std::string& body,
                                              std::string& timestamp) {
  bool has_access = user_service.has_access(id_user, id_channel);
  if (!has_access) {
    std::cerr << "INVALID_USER User has no permission to SEND_MESSAGE to this "
                 "channel\n";
    throw UnauthorizedError(
        "INVALID_USER User has no permission to SEND_MESSAGE to this channel");
  }

  int64_t new_id_message =
      message_service.create_message(id_user, id_channel, body, timestamp);
  this->broadcast_new_message(id_channel, new_id_message, id_user, body,
                              timestamp);
}

void MessageController::broadcast_new_message(const int64_t id_channel,
                                              const int64_t id_message,
                                              const int64_t id_user,
                                              const std::string& body,
                                              const std::string& timestamp) {
  ServerSend::SendMessageResponse broadcast;
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  broadcast.id_channel = id_channel;
  broadcast.message =
      create_message_struct(id_message, id_user, body, timestamp);
  std::string broadcast_json_str =
      JsonHelpers::ServerSend::to_json(broadcast).dump();
  std::unordered_set<int64_t> participants =
      user_service.get_users_by_channel(id_channel);
  ws_manager.broadcast_to_users(participants, broadcast_json_str);
}

void MessageController::send_history(crow::websocket::connection& conn,
                                     int64_t id_user,
                                     const crow::json::rvalue& json_msg) {
  try {
    std::optional<::ClientSend::ChannelHistoryRequest> req =
        JsonHelpers::ClientSend::parse_request_channel_history(json_msg);

    if (!req.has_value()) {
      throw BadRequestError("Invalid REQUEST_CHANNEL_HISTORY format");
      // send_error(conn, "INVALID_FORMAT",
      //            "Invalid REQUEST_CHANNEL_HISTORY format");
      // return;
    }

    std::cout << "[HISTORY] User " << id_user << " -> Channel "
              << req->id_channel << "\n";
    std::vector<ServerSend::Message> messages =
        message_service.get_messages_history_from_channel(
            req->id_channel, req->before_id_message, req->limit);

    this->send_history_response(conn, req->id_channel, messages, req->limit);
  } catch (const WizzManiaError& e) {
    WizzManiaError::send_ws_error(conn, e);
  }
}

void MessageController::send_history_response(
    crow::websocket::connection& conn, int64_t id_channel,
    std::vector<ServerSend::Message>& messages, int limit) {
  ServerSend::ChannelHistoryResponse res;
  res.type = WizzMania::MessageType::CHANNEL_HISTORY;
  res.id_channel = id_channel;
  res.messages = messages;
  res.has_more = messages.size() == limit;
  conn.send_text(JsonHelpers::ServerSend::to_json(res).dump());
}