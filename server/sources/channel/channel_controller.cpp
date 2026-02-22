#include "channel_controller.hpp"

// void ChannelController::create_channel(crow::websocket::connection& conn,
//                                        int64_t id_user,
//                                        const crow::json::rvalue& json_msg) {
//   std::optional<::ClientSend::CreateChannelRequest> req =
//       JsonHelpers::ClientSend::parse_create_channel(json_msg);
//   if (!req.has_value()) {
//     send_error(conn, "INVALID_FORMAT", "Invalid CREATE_CHANNEL format");
//     return;
//   }

//   // Step 1 : get users id, send error with username if id not found !
//   std::unordered_set<int64_t> participants;
//   try {
//     for (const std::string& username : req->usernames) {
//       int64_t id_user = user_service.get_id_user(username);
//       participants.insert(id_user);
//     }
//   } catch (const WizzManiaError& e) {
//     return send_error(conn, "INVALID_USERNAME", e.get_message());
//   }

//   try {
//     // Step 2 : if all ids ok, create channel title (concat all name,
//     truncate std::string title = req->title;
//     channel_service.generate_title(title, req->usernames);

//     std::string created_at = Utils::get_timestamp();
//     int64_t id_channel = channel_service.create_channel(
//         id_user, title, created_at, participants);

//     std::vector<ServerSend::Contact> contacts =
//         user_service.get_contacts_from_channel(id_channel);

//     ServerSend::ChannelInvitation invitation_message =
//         create_invitation_struct(id_channel, id_user, contacts, title);

//     invitation_controller.broadcast_invitation_notification(participants,
//                                                             invitation_message);

//     ServerSend::CreateChannelResponse resp;
//     resp.type = WizzMania::MessageType::CHANNEL_CREATED;
//     resp.id_channel = id_channel;
//     resp.already_existed = false;  // TODO !! MAKE A CHECK IF A CHANNEL WITH
//                                    // THESE PARTICIPANT ALREADY EXIST OR NOT
//     resp.channel =
//         create_empty_channel_info_struct(id_channel, id_user, contacts,
//         title);
//     ws_manager.send_to_user(id_user,
//                             JsonHelpers::ServerSend::to_json(resp).dump());

//   } catch (const WizzManiaError& e) {
//     return send_error(conn, "INTERNAL ERROR", e.get_message());
//   }
// }

crow::response ChannelController::create_channel(int64_t id_user,
                                                 const crow::request& req) {
  // std::optional<::ClientSend::CreateChannelRequest> req =
  //     JsonHelpers::ClientSend::parse_create_channel(json_msg);
  // if (!req.has_value()) {
  //   send_error(conn, "INVALID_FORMAT", "Invalid CREATE_CHANNEL format");
  //   return;
  // }
  crow::json::rvalue body = crow::json::load(req.body);
  if (!body || !body.has("usernames")) {
    BadRequestError error = BadRequestError("Invalid CREATE_CHANNEL format");
    return WizzManiaError::send_http_error(error.get_code(), error.get_message());
  }

  std::unordered_set<std::string> usernames;
  for (const auto& participant : body["usernames"]) {
    usernames.insert(participant.s());
  }
  std::string title = body.has("title") ? std::string(body["title"].s()) : "";
  // Step 1 : get users id, send error with username if id not found !
  std::unordered_set<int64_t> participants;
  try {
    for (const std::string& username : usernames) {
      int64_t id_user = user_service.get_id_user(username);
      participants.insert(id_user);
    }
  } catch (const WizzManiaError& e) {
    BadRequestError error = BadRequestError("Invalid username");
    return WizzManiaError::send_http_error(error.get_code(), error.get_message());
    // return send_error(conn, "INVALID_USERNAME", e.get_message());
  }

  try {
    // Step 2 : if all ids ok, create channel title (concat all name, truncate
    // std::string title = req->title;
    channel_service.generate_title(title, usernames, id_user);

    std::string created_at = Utils::get_timestamp();
    int64_t id_channel = channel_service.create_channel(
        id_user, title, created_at, participants);

    std::vector<ServerSend::Contact> contacts =
        user_service.get_contacts_from_channel(id_channel);

    ServerSend::ChannelInvitation invitation_message =
        create_invitation_struct(id_channel, id_user, contacts, title);

    invitation_controller.broadcast_invitation_notification(participants,
                                                            invitation_message);

    ServerSend::CreateChannelResponse resp;
    resp.type = WizzMania::MessageType::CHANNEL_CREATED;
    resp.id_channel = id_channel;
    resp.already_existed = false;  // TODO !! MAKE A CHECK IF A CHANNEL WITH
                                   // THESE PARTICIPANT ALREADY EXIST OR NOT
    resp.channel =
        create_empty_channel_info_struct(id_channel, id_user, contacts, title);
    // ws_manager.send_to_user(id_user,
    //                         JsonHelpers::ServerSend::to_json(resp).dump());

    return crow::response(201, JsonHelpers::ServerSend::to_json(resp).dump());

  } catch (const WizzManiaError& e) {
    // return send_error(conn, "INTERNAL ERROR", e.get_message());
    return WizzManiaError::send_http_error(e.get_code(), e.get_message());
  }
}
