#include "message_handler.hpp"

void MessageHandler::send_message(crow::websocket::connection& conn,
                                  int64_t id_user,
                                  const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::SendMessageRequest> req =
      JsonHelpers::ClientSend::parse_send_message(json_msg);
  if (!req.has_value()) {
    this->send_error(conn, "INVALID_FORMAT", "Invalid SEND_MESSAGE format");
    return;
  }

  std::cout << "[MSG] User " << id_user << " -> Channel " << req->id_channel
            << ": " << req->body << "\n";

  // TODO: Validate channel access

  std::string body = req->body;
  int64_t id_channel = req->id_channel;
  std::string timestamp = get_timestamp();

  std::optional<int64_t> id_message_opt =
      db.save_message(id_user, id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message coul not be save in db\n";
    return;
  }
  int64_t id_message = id_message_opt.value();

  ServerSend::NewMessageBroadcast broadcast;
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  broadcast.id_channel = id_channel;
  broadcast.message = create_message(id_message, id_user, body, timestamp);
  // broadcast.message.id_message = id_message;
  // broadcast.message.id_sender = id_user;
  // broadcast.message.body = body;
  // broadcast.message.timestamp = timestamp;
  // broadcast.message.is_system = id_user == 1;

  std::string json_str = JsonHelpers::ServerSend::to_json(broadcast).dump();
  std::unordered_set<int64_t> participants =
      db.get_channel_participants(id_channel);
  ws_manager.broadcast_to_users(participants, json_str);
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

void MessageHandler::auth_error(crow::websocket::connection& conn,
                                const std::string& message) {
  std::cout << "[WS]" << message << "\n";
  conn.close(message);
}

void MessageHandler::auth_success(crow::websocket::connection& conn,
                                  const int64_t validated_id_user) {
  std::cout << "[WS] ✅ User " << validated_id_user << " authenticated!\n";

  ws_manager.add_user(validated_id_user, &conn);

  AuthMessages::WSAuthResponse auth_resp;
  auth_resp.type =
      WizzMania::MessageType::WS_AUTH_SUCCESS;  // Explicitly set type
  auth_resp.message = "Authentication successful";
  auth_resp.id_user = validated_id_user;
  conn.send_text(JsonHelpers::Auth::to_json(auth_resp).dump());
}

void MessageHandler::authenticate_ws(crow::websocket::connection& conn,
                                     const crow::json::rvalue& json_msg) {
  if (ws_manager.is_authenticated(&conn)) {
    std::cout << "[WS] User already authenticated\n";
    return;
  }

  std::cout << "[WS] Processing authentication request\n";

  std::optional<::AuthMessages::WSAuthRequest> auth_req =
      JsonHelpers::Auth::parse_ws_auth_request(json_msg);

  if (!auth_req.has_value()) {
    this->auth_error(conn, "Invalid authentication format");
    return;
  }

  std::optional<int64_t> validated_id_user =
      Auth::validateToken(auth_req->token);

  if (!validated_id_user.has_value()) {
    this->auth_error(conn, "Invalid token");
    return;
  }

  this->auth_success(conn, validated_id_user.value());
  this->initial_data(conn);

  return;
}

void MessageHandler::initial_data(crow::websocket::connection& conn) {
  std::optional<int64_t> id_user_opt = ws_manager.get_user_id(&conn);
  if (!id_user_opt.has_value()) {
    std::cerr << "[INIT] Error: user not found for connection\n";
    return;
  }
  int64_t id_user = id_user_opt.value();

  std::cout << "[INIT] Sending initial data to user " << id_user << "\n";

  ServerSend::InitialDataResponse init_data = db.get_initial_data(id_user);
  init_data.type = WizzMania::MessageType::INITIAL_DATA;
  if (init_data.contacts.empty()) {
    std::cout << "[INIT] Warning: No contacts found for user " << id_user
              << "\n";
  }
  std::string json_str = JsonHelpers::ServerSend::to_json(init_data).dump();
  conn.send_text(json_str);

  std::cout << "[INIT] Sent initial data: " << init_data.contacts.size()
            << " contacts, " << init_data.channels.size() << " channels, "
            << init_data.invitations.size() << " invitations\n";
}

void MessageHandler::create_channel(crow::websocket::connection& conn,
                                    int64_t id_user,
                                    const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::CreateChannelRequest> req =
      JsonHelpers::ClientSend::parse_create_channel(json_msg);
  if (!req.has_value()) {
    send_error(conn, "INVALID_FORMAT", "Invalid CREATE_CHANNEL format");
    return;
  }

  // std::cout << "[CHANNEL] User " << id_user << " creating with "
  //           << req->participant_ids.size() << " participants\n";

  // TODO: Create channel
}

void MessageHandler::send_history(crow::websocket::connection& conn,
                                  int64_t id_user,
                                  const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::RequestChannelHistoryRequest> req =
      JsonHelpers::ClientSend::parse_request_channel_history(json_msg);

  if (!req.has_value()) {
    this->send_error(conn, "INVALID_FORMAT",
                     "Invalid REQUEST_CHANNEL_HISTORY format");
    return;
  }

  std::cout << "[HISTORY] User " << id_user << " -> Channel " << req->id_channel
            << "\n";

  std::vector<ServerSend::Message> messages = db.get_channel_history(
      req->id_channel, req->before_id_message, req->limit);
  if (messages.empty()) {
    std::cerr << "[HISTORY] : messages could not be retrieve in db\n";
    // actually possible to have no history, but I want to have a log about it
  }

  ServerSend::ChannelHistoryResponse res;
  res.type = WizzMania::MessageType::CHANNEL_HISTORY;
  res.id_channel = req->id_channel;
  res.messages = messages;
  res.has_more = messages.size() == req->limit;
  conn.send_text(JsonHelpers::ServerSend::to_json(res).dump());
}