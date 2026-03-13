#include "message_controller.hpp"

// void MessageController::send_message(crow::websocket::connection& conn,
//                                      int64_t id_user,
//                                      const crow::json::rvalue& json_msg) {
//   std::optional<::ClientSend::SendMessageRequest> req =
//       JsonHelpers::ClientSendHelpers::parse_send_message(json_msg);
//   try {
//     if (!req.has_value()) {
//       throw BadRequestError("Invalid SEND_MESSAGE format");
//     }

//     std::cout << "[MSG] User " << id_user << " -> Channel " <<
//     req->id_channel
//               << ": " << req->body << "\n";

//     int64_t id_channel = req->id_channel;
//     bool has_access = user_service.has_access(id_user, id_channel);
//     if (!has_access) {
//       throw UnauthorizedError(
//           "User has no permission to SEND_MESSAGE to this channel");
//     }

//     std::string body = req->body;
//     std::string timestamp = Utils::get_timestamp();

//     int64_t new_id_message =
//         message_service.create_message(id_user, id_channel, body, timestamp);
//     this->broadcast_new_message(id_channel, new_id_message, id_user, body,
//                                 timestamp);
//   } catch (const WizzManiaError& e) {
//     // return send_error(conn, "INTERNAL_ERROR", e.get_message());
//     WizzManiaError::send_ws_error(conn, e);
//   }
// }

crow::response MessageController::send_message(const crow::request& req,
                                               int64_t id_user,
                                               int64_t id_channel,
                                               const std::string& token) {
  crow::json::rvalue body = crow::json::load(req.body);
  if (!body || !body.has("body")) {
    throw BadRequestError("Invalid SEND_MESSAGE format");
  }

  if (!user_service.has_access(id_user, id_channel)) {
    throw UnauthorizedError(
        "User has no permission to SEND_MESSAGE to this channel");
  }

  std::string msg_body = std::string(body["body"].s());
  std::string timestamp = Utils::get_timestamp();

  std::cout << "[MSG] User " << id_user << " -> Channel " << id_channel << ": "
            << msg_body << "\n";

  int64_t new_id_message =
      message_service.create_message(id_user, id_channel, msg_body, timestamp);

  // need to evict current sessions !!!
  broadcast_new_message(id_channel, new_id_message, id_user, msg_body,
                        timestamp, token);

  ServerSend::SendMessageResponse broadcast;
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  broadcast.id_channel = id_channel;
  broadcast.message =
      Structure::create_message_struct(new_id_message, id_user, msg_body, timestamp);
  std::string broadcast_json_str =
      JsonHelpers::ServerSendHelpers::to_json(broadcast).dump();
  // crow::json::wvalue resp;
  // resp["id_message"] = new_id_message;
  // resp["timestamp"] = timestamp;

  crow::response res(201, broadcast_json_str);
  res.add_header("Content-Type", "application/json");
  return res;
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
                              timestamp, "");
}

void MessageController::broadcast_new_message(const int64_t id_channel,
                                              const int64_t id_message,
                                              const int64_t id_user,
                                              const std::string& body,
                                              const std::string& timestamp,
                                              const std::string& token) {
  ServerSend::SendMessageResponse broadcast;
  broadcast.type = WizzMania::MessageType::NEW_MESSAGE;
  broadcast.id_channel = id_channel;
  broadcast.message =
      Structure::create_message_struct(id_message, id_user, body, timestamp);
  std::string broadcast_json_str =
      JsonHelpers::ServerSendHelpers::to_json(broadcast).dump();
  std::unordered_set<int64_t> participants =
      user_service.get_users_by_channel(id_channel);

  if (token.empty()) {
    ws_manager.broadcast_to_users(participants, broadcast_json_str);
  } else {
    ws_manager.send_to_user_except(id_user, broadcast_json_str, token);
    participants.erase(id_user);

    ws_manager.broadcast_to_users(participants, broadcast_json_str);
  }
}

// crow::response MessageController::send_message_http(const crow::request& req,
//                                                     int64_t id_user,
//                                                     int64_t id_channel) {
//   crow::json::rvalue json_msg = crow::json::load(req.body);
//   if (!json_msg) {
//     throw BadRequestError("Invalid SEND_MESSAGE format");
//   }

//   std::optional<ClientSend::SendMessageRequest> parsed =
//       JsonHelpers::ClientSendHelpers::parse_send_message(json_msg);
//   if (!parsed.has_value()) {
//     throw BadRequestError("Invalid SEND_MESSAGE format");
//   }

//   if (parsed->id_channel != id_channel) {
//     throw BadRequestError("Channel mismatch in SEND_MESSAGE payload");
//   }

//   if (!user_service.has_access(id_user, id_channel)) {
//     throw UnauthorizedError(
//         "User has no permission to SEND_MESSAGE to this channel");
//   }

//   const std::string timestamp = Utils::get_timestamp();
//   const std::string body = parsed->body;
//   const int64_t new_id_message =
//       message_service.create_message(id_user, id_channel, body, timestamp);

//   ServerSend::SendMessageResponse response;
//   response.type = WizzMania::MessageType::NEW_MESSAGE;
//   response.id_channel = id_channel;
//   response.message = Structure::create_message_struct(new_id_message,
//   id_user,
//                                                       body, timestamp);

//   const std::string response_json =
//       JsonHelpers::ServerSendHelpers::to_json(response).dump();

//   broadcast_new_message(id_channel, new_id_message, id_user, body, timestamp,
//                         id_user);

//   crow::response http_response(200, response_json);
//   http_response.add_header("Content-Type", "application/json");
//   return http_response;
// }

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

// === Mark as Read === //
// void MessageController::mark_as_read(crow::websocket::connection& conn,
//                                      int64_t id_user,
//                                      const crow::json::rvalue& json_msg) {
//   try {
//     // parse the json from client
//     std::optional<ClientSend::MarkAsRead> mark =
//         JsonHelpers::ClientSendHelpers::parse_mark_as_read(json_msg);

//     // if (!mark.has_value()) {
//     if (!mark.has_value()) {
//       throw BadRequestError("Invalid MARK_AS_READ format");
//     }

//     std::cout << "[MARK_AS_READ] User " << id_user << " -> Channel "
//               << mark->id_channel << " read up to message "
//               << mark->last_id_message << "\n";
//     // << mark->id_channel << " read up to message "
//     // << mark->last_id_message << "\n";

//     // check if user has access to this channel
//     // bool has_access = user_service.has_access(id_user, mark->id_channel);
//     // check if user has access to this channel
//     bool has_access = user_service.has_access(id_user, mark->id_channel);
//     if (!has_access) {
//       throw UnauthorizedError(
//           "User has no permission to MARK_AS_READ in this channel");
//     }

//     // update last_read_id_message in db
//     message_service.mark_as_read(id_user, mark->id_channel,
//                                  mark->last_id_message);

//     // get updated unread count from db
//     mark->unread_count = db.get_unread_count(id_user, mark->id_channel);

//     // send response to all connected devices of this user
//     ws_manager.send_to_user(
//         id_user,
//         JsonHelpers::ClientSendHelpers::to_json(mark.value()).dump());

//   } catch (const WizzManiaError& e) {
//     WizzManiaError::send_ws_error(conn, e);
//   }
// }

crow::response MessageController::mark_as_read(const crow::request& req,
                                               int64_t id_user,
                                               int64_t id_channel,
                                               const std::string& token) {
  crow::json::rvalue body = crow::json::load(req.body);
  if (!body || !body.has("last_id_message")) {
    throw BadRequestError("Invalid MARK_AS_READ format");
  }

  int64_t last_id_message = body["last_id_message"].i();

  std::cout << "[MARK_AS_READ] User " << id_user << " -> Channel " << id_channel
            << " read up to message " << last_id_message << "\n";

  if (!user_service.has_access(id_user, id_channel)) {
    throw UnauthorizedError(
        "User has no permission to MARK_AS_READ in this channel");
  }

  message_service.mark_as_read(id_user, id_channel, last_id_message);

  int64_t unread_count = db.get_unread_count(id_user, id_channel);

  // Notify other connected devices of this user via WS
  ClientSend::MarkAsRead mark;
  mark.type = WizzMania::MessageType::MARK_AS_READ;
  mark.id_channel = id_channel;
  mark.last_id_message = last_id_message;
  mark.unread_count = unread_count;
  std::string mark_json = JsonHelpers::ClientSendHelpers::to_json(mark).dump();
  ws_manager.send_to_user_except(id_user, mark_json, token);

  crow::response res(200, mark_json);
  res.add_header("Content-Type", "application/json");
  return res;
}

// ===== POST /channels/<id>/wizz =====
crow::response MessageController::wizz(int64_t id_user, int64_t id_channel,
                                       const std::string& token) {
  std::cout << "[WIZZ] User " << id_user << " in channel " << id_channel
            << "\n";

  if (!user_service.has_access(id_user, id_channel)) {
    throw ForbiddenError("No access to this channel");
  }

  ServerSend::WizzNotification notif;
  notif.type = WizzMania::MessageType::WIZZ;
  notif.id_channel = id_channel;
  notif.id_user = id_user;

  std::string json = JsonHelpers::ServerSendHelpers::to_json(notif).dump();

  std::unordered_set<int64_t> participants =
      user_service.get_users_by_channel(id_channel);

  ws_manager.send_to_user_except(id_user, json, token);
  
  participants.erase(id_user);
  ws_manager.broadcast_to_users(participants, json);

  crow::response res(200, json);
  res.add_header("Content-Type", "application/json");
  return res;
}