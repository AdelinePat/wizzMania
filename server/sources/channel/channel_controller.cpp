#include "channel_controller.hpp"

crow::response ChannelController::create_channel(int64_t id_user,
                                                 const crow::request& req) {
  crow::json::rvalue body = crow::json::load(req.body);
  if (!body || !body.has("usernames")) {
    BadRequestError error = BadRequestError("Invalid CREATE_CHANNEL format");
    return WizzManiaError::send_http_error(error.get_code(),
                                           error.get_message());
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
      int64_t current_id_user = user_service.get_id_user(username);
      participants.insert(current_id_user);
    }
    participants.insert(id_user);

  } catch (const WizzManiaError& e) {
    BadRequestError error = BadRequestError("Invalid username");
    return WizzManiaError::send_http_error(error.get_code(),
                                           error.get_message());
  }

  try {
    std::optional<int64_t> existing =
        channel_service.find_existing_channel(participants);

    ServerSend::CreateChannelResponse resp;

    if (existing.has_value()) {
      resp.type = WizzMania::MessageType::CHANNEL_CREATED;
      resp.id_channel = existing.value();
      resp.already_existed = true;
      return crow::response(
          409, JsonHelpers::ServerSendHelpers::to_json(resp).dump());
    }

    channel_service.generate_title(title, usernames, id_user);

    std::string created_at = Utils::get_timestamp();

    // ALED
    participants.erase(id_user);
    int64_t id_channel = channel_service.create_channel(
        id_user, title, created_at, participants);

    std::vector<ServerSend::Contact> contacts =
        user_service.get_contacts_from_channel(id_channel);

    ServerSend::ChannelInvitation invitation_message =
        Structure::create_invitation_struct(id_channel, id_user, contacts,
                                            title);

    invitation_controller.broadcast_invitation_notification(participants,
                                                            invitation_message);

    // ServerSend::CreateChannelResponse resp;
    resp.type = WizzMania::MessageType::CHANNEL_CREATED;
    resp.id_channel = id_channel;
    resp.already_existed = false;

    resp.channel = Structure::create_empty_channel_info_struct(
        id_channel, id_user, contacts, title);

    return crow::response(201,
                          JsonHelpers::ServerSendHelpers::to_json(resp).dump());

  } catch (const WizzManiaError& e) {
    return WizzManiaError::send_http_error(e.get_code(), e.get_message());
  }
}

crow::response ChannelController::leave_channel(const crow::request& req,
                                                int64_t id_user,
                                                int64_t id_channel) {
  try {
    channel_service.leave_channel(id_user, id_channel);

    // notify remaining participants via WS
    ServerSend::UserLeftNotification notif;
    notif.type = WizzMania::MessageType::USER_LEFT;
    notif.id_channel = id_channel;
    notif.id_user = id_user;

    std::unordered_set<int64_t> remaining =
        user_service.get_users_by_channel(id_channel);
    ws_manager.broadcast_to_users(
        remaining, JsonHelpers::ServerSendHelpers::to_json(notif).dump());

    return crow::response(204);  // ok with no body
  } catch (const WizzManiaError& e) {
    return WizzManiaError::send_http_error(e.get_code(), e.get_message());
  }
}