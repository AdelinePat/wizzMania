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

  int64_t id_channel = req->id_channel;

  bool is_system_user = (id_user == 1);
  bool has_channel_access = db.has_channel_access(id_user, id_channel);
  if (!is_system_user && !has_channel_access) {
    std::cerr << "[INIT] Error: user has no access to this channel !\n";
    send_error(conn, "INVALID_USER",
               "User has no permission to SEND_MESSAGE to this channel");
    return;
  }

  std::string body = req->body;
  std::string timestamp = get_timestamp();

  std::optional<int64_t> id_message_opt =
      db.save_message(id_user, id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    return;
  }
  int64_t id_message = id_message_opt.value();
  this->broadcast_new_message(id_channel, id_message, id_user, body, timestamp);

  // ServerSend::SendMessageResponse broadcast;
  // broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  // broadcast.id_channel = id_channel;
  // broadcast.message = create_message(id_message, id_user, body, timestamp);

  // std::string json_str = JsonHelpers::ServerSend::to_json(broadcast).dump();
  // std::unordered_set<int64_t> participants =
  //     db.get_channel_participants(id_channel);
  // ws_manager.broadcast_to_users(participants, json_str);
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
  std::optional<::ClientSend::ChannelHistoryRequest> req =
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

void MessageHandler::accept_invitation(crow::websocket::connection& conn,
                                       int64_t id_user,
                                       const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::AcceptInvitationRequest> req =
      JsonHelpers::ClientSend::parse_accept_invitation(json_msg);

  if (!req.has_value()) {
    std::cerr << "[INVALID_FORMAT] : Invalid ACCEPT_INVITATION format\n";
    return;
  }

  std::cout << "[INVITATION] User " << id_user << " -> accepts to Channel "
            << req->id_channel << "\n";

  bool accepted = db.accept_invitation(id_user, req->id_channel);
  if (!accepted) {
    this->send_error(conn, "[INVITATION ERROR]",
                     "Couldn't accept the invitation");
    return;
  }

  // Get channel info for new participant (the one accepting the request)
  ServerSend::AcceptInvitationResponse resp;
  resp.type = WizzMania::MessageType::INVITATION_ACCEPTED;
  resp.channel = db.get_channel(id_user, req->id_channel);
  std::string channel_info_str = JsonHelpers::ServerSend::to_json(resp).dump();
  // conn.send_text(channel_info_str);
  ws_manager.send_to_user(id_user, channel_info_str);

  // send new participant list to all current participant of channel
  broadcast_joined_notification(id_user, req->id_channel,
                                resp.channel.participants);

  std::string body = "User @" + std::to_string(id_user) + " joined the chat!";

  std::string timestamp = get_timestamp();
  std::optional<int64_t> id_message_opt =
      db.save_message(1, req->id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    return;
  }
  int64_t id_message = id_message_opt.value();
  this->broadcast_new_message(req->id_channel, id_message, 1, body, timestamp);
}

void MessageHandler::reject_invitation(crow::websocket::connection& conn,
                                       int64_t id_user,
                                       const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::RejectInvitationRequest> req =
      JsonHelpers::ClientSend::parse_reject_invitation(json_msg);

  if (!req.has_value()) {
    std::cerr << "[INVALID_FORMAT] : Invalid REJECT_INVITATION format\n";
    return;
  }

  std::optional<int64_t> id_creator_opt =
      db.get_channel_creator(req->id_channel);

  if (!id_creator_opt.has_value() || id_creator_opt.value() == id_user) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    // SEND ERROR TO CLIENT (conn) AND TELL THEM AN ERROR OCCURED OR SOMETHING
    return;
  }
  int64_t id_creator = id_creator_opt.value();

  bool rejected = db.reject_invitation(id_user, req->id_channel);
  if (!rejected) {
    this->send_error(conn, "[INVITATION ERROR]",
                     "Couldn't reject the invitation");
    return;
  }
  std::cout << "[INVITATION] User " << id_user << " -> declined the invitation "
            << req->id_channel << "\n";

  ServerSend::RejectInvitationResponse resp;
  resp.type = WizzMania::MessageType::INVITATION_REJECTED;
  resp.id_channel = req->id_channel;
  std::optional<ServerSend::Contact> rejecter_opt = db.get_contact(id_user);
  resp.contact = rejecter_opt.value();

  std::string resp_json = JsonHelpers::ServerSend::to_json(resp).dump();

  ws_manager.send_to_user(id_user, resp_json);
  ws_manager.send_to_user(id_creator, resp_json);

  // ws_manager.send_to_user(id_creator, )

  // ServerSend::AcceptInvitationResponse resp;
  // resp.type = WizzMania::MessageType::INVITATION_ACCEPTED;
  // resp.channel = db.get_channel(id_user, req->id_channel);
  // std::string channel_info_str =
  // JsonHelpers::ServerSend::to_json(resp).dump();
  // conn.send_text(channel_info_str);

  // broadcast_joined_notification(id_user, req->id_channel,
  //                               resp.channel.participants);

  std::string body = "User @" + std::to_string(id_user) + " rejected the chat!";

  std::string timestamp = get_timestamp();
  std::optional<int64_t> id_message_opt =
      db.save_message(1, req->id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    return;
  }
  int64_t id_message = id_message_opt.value();
  this->broadcast_new_message(req->id_channel, id_message, 1, body, timestamp);
}

void MessageHandler::broadcast_new_message(const int64_t id_channel,
                                           const int64_t id_message,
                                           const int64_t id_user,
                                           const std::string& body,
                                           const std::string& timestamp) {
  ServerSend::SendMessageResponse broadcast;
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  broadcast.id_channel = id_channel;
  broadcast.message = create_message(id_message, id_user, body, timestamp);
  std::string broadcast_json_str =
      JsonHelpers::ServerSend::to_json(broadcast).dump();
  std::unordered_set<int64_t> participants =
      db.get_channel_participants(id_channel);
  ws_manager.broadcast_to_users(participants, broadcast_json_str);
}

void MessageHandler::broadcast_joined_notification(
    const int64_t id_user, const int64_t id_channel,
    std::vector<ServerSend::Contact>& participants) {
  ServerSend::UserJoinedNotification joined_notification;
  joined_notification.type = WizzMania::MessageType::USER_JOINED;
  joined_notification.id_channel = id_channel;
  joined_notification.contact.id_user = id_user;
  std::string new_username = "";
  for (const ServerSend::Contact& participant : participants) {
    if (participant.id_user == id_user) {
      new_username = participant.username;
      break;
    }
  }
  joined_notification.contact.username = new_username;

  std::string joined_json =
      JsonHelpers::ServerSend::to_json(joined_notification).dump();
  std::unordered_set<int64_t> participants_set =
      db.get_channel_participants(id_channel);

  participants_set.erase(id_user);
  ws_manager.broadcast_to_users(participants_set, joined_json);
}