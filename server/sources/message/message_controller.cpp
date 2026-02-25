#include "message_controller.hpp"

void MessageController::send_message(crow::websocket::connection& conn,
                                     int64_t id_user,
                                     const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::SendMessageRequest> req =
      JsonHelpers::ClientSendHelpers::parse_send_message(json_msg);
  try {
    if (!req.has_value()) {
      throw BadRequestError("Invalid SEND_MESSAGE format");
    }

    std::cout << "[MSG] User " << id_user << " -> Channel " << req->id_channel
              << ": " << req->body << "\n";

    int64_t id_channel = req->id_channel;
    bool has_access = user_service.has_access(id_user, id_channel);
    if (!has_access) {
      throw UnauthorizedError(
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
      Structure::create_message_struct(id_message, id_user, body, timestamp);
  std::string broadcast_json_str =
      JsonHelpers::ServerSendHelpers::to_json(broadcast).dump();
  std::unordered_set<int64_t> participants =
      user_service.get_users_by_channel(id_channel);
  ws_manager.broadcast_to_users(participants, broadcast_json_str);
}

// switch to HTTP
crow::response MessageController::get_history(const crow::request& req,
                                              int64_t id_user,
                                              int64_t id_channel) {
  // crow::json::rvalue body = crow::json::load(req.body);
  // if (!body || !body.has("usernames")) {
  //   throw BadRequestError("Invalid HISTORY_REQUEST format");
  // }

  // int64_t id_channel = req->id_channel;
  bool has_access = user_service.has_access(id_user, id_channel);
  if (!has_access) {
    throw UnauthorizedError(
        "User has no permission to GET_HISTORY for this channel");
  }

  int64_t before_id_message = 0;
  int limit = 50;
  if (req.url_params.get("before_id")) {
    before_id_message = std::stoll(req.url_params.get("before_id"));
  }
  if (req.url_params.get("limit")) {
    limit = std::stoi(req.url_params.get("limit"));
    if (limit > 100) {
      limit = 100;
    }
  }

  std::cout << "[HISTORY] User " << id_user << " -> Channel " << id_channel
            << "\n";
  std::vector<ServerSend::Message> messages =
      message_service.get_messages_history_from_channel(
          id_channel, before_id_message, limit);
  ServerSend::ChannelHistoryResponse resp =
      Structure::create_history_response_struct(id_channel, messages, limit);

  std::string history_str =
      JsonHelpers::ServerSendHelpers::to_json(resp).dump();

  crow::response response(200, history_str);
  response.add_header("Content-Type", "application/json");
  return response;
}

void MessageController::send_history(crow::websocket::connection& conn,
                                     int64_t id_user,
                                     const crow::json::rvalue& json_msg) {
  try {
    std::optional<::ClientSend::ChannelHistoryRequest> req =
        JsonHelpers::ClientSendHelpers::parse_request_channel_history(json_msg);

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
  conn.send_text(JsonHelpers::ServerSendHelpers::to_json(res).dump());
}

// === Mark as Read === //
void MessageController::mark_as_read(crow::websocket::connection& conn,
                                     int64_t id_user,
                                     const crow::json::rvalue& json_msg) {
  try {
    // parse the json from client
    std::optional<ClientSend::MarkAsRead> mark =
        JsonHelpers::ClientSendHelpers::parse_mark_as_read(json_msg);

    if (!mark.has_value()) {
      throw BadRequestError("Invalid MARK_AS_READ format");
    }

    std::cout << "[MARK_AS_READ] User " << id_user << " -> Channel "
              << mark->id_channel << " read up to message "
              << mark->last_id_message << "\n";

    // check if user has access to this channel
    bool has_access = user_service.has_access(id_user, mark->id_channel);
    if (!has_access) {
      throw UnauthorizedError(
          "User has no permission to MARK_AS_READ in this channel");
    }

    // update last_read_id_message in db
    message_service.mark_as_read(id_user, mark->id_channel,
                                 mark->last_id_message);

    // get updated unread count from db
    mark->unread_count = db.get_unread_count(id_user, mark->id_channel);

    // send response to all connected devices of this user
    ws_manager.send_to_user(
        id_user, JsonHelpers::ClientSendHelpers::to_json(mark.value()).dump());

  } catch (const WizzManiaError& e) {
    WizzManiaError::send_ws_error(conn, e);
  }
}
