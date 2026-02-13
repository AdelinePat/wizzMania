#include "message_handler.hpp"

void MessageHandler::send_message(crow::websocket::connection& conn,
                                  int64_t id_user,
                                  const crow::json::rvalue& json_msg) {
  //    Shouldn't this part be inside this function???

  std::optional<::ClientSend::SendMessageRequest> req =
      JsonHelpers::ClientSend::parse_send_message(json_msg);
  if (!req.has_value()) {
    this->send_error(conn, "INVALID_FORMAT", "Invalid SEND_MESSAGE format");
    return;
  }

  std::cout << "[MSG] User " << id_user << " -> Channel " << req->channel_id
            << ": " << req->body << "\n";

  // TODO: Validate channel access
  // TODO: Store message in database

  // TEMP : ECHO MSG BACK
  ServerSend::NewMessageBroadcast broadcast;             // outer struct
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;  // ✅ this exists here
  broadcast.channel_id = req->channel_id;

  // fill the inner message
  broadcast.message.message_id = 0;  // fake id for now TODO get from DB!!
  broadcast.message.sender_id = id_user;
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
  int64_t id_user = ws_manager.get_user_id(&conn);
  std::cout << "[INIT] Sending initial data to user " << id_user << "\n";

  ServerSend::InitialDataResponse init_data;
  init_data.type = WizzMania::MessageType::INITIAL_DATA;
  init_data.contacts = db.get_contact(id_user);
  // std::vector<ChannelInfo> channels;
  if (!init_data.contacts.empty()) {
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