#include "invitation_controller.hpp"

void InvitationController::accept_invitation(
    crow::websocket::connection& conn, int64_t id_user,
    const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::AcceptInvitationRequest> req =
      JsonHelpers::ClientSend::parse_accept_invitation(json_msg);

  if (!req.has_value()) {
    std::cout << "[INVALID_FORMAT] : Invalid ACCEPT_INVITATION format\n";
    return;
  }

  std::cout << "[INVITATION] User " << id_user << " -> accepts to Channel "
            << req->id_channel << "\n";

  std::string responded_at = get_timestamp();

  invitation_service.accept_invitation(id_user, req->id_channel, responded_at);

  // Get channel info for new participant (the one accepting the request)
  ServerSend::AcceptInvitationResponse resp;
  resp.type = WizzMania::MessageType::INVITATION_ACCEPTED;

  resp.channel = channel_service.get_channel(id_user, req->id_channel);

  std::string channel_info_str = JsonHelpers::ServerSend::to_json(resp).dump();

  ws_manager.send_to_user(id_user, channel_info_str);

  // send new participant list to all current participant of channel
  broadcast_joined_notification(id_user, req->id_channel,
                                resp.channel.participants);

  std::string body = "User @" + std::to_string(id_user) + " joined the chat!";
  try {
    message_controller.send_message_internal(1, req->id_channel, body,
                                             responded_at);
  } catch (const WsError& e) {
    return send_error(conn, "INTERNAL_ERROR", e.get_message());
  }
}

// Reject invitation
void InvitationController::reject_invitation(
    crow::websocket::connection& conn, int64_t id_user,
    const crow::json::rvalue& json_msg) {
  std::optional<::ClientSend::RejectInvitationRequest> req =
      JsonHelpers::ClientSend::parse_reject_invitation(json_msg);
  if (!req.has_value()) {
    std::cout << "[INVALID_FORMAT] : Invalid REJECT_INVITATION format\n";
    return;
  }

  try {
    int64_t id_creator =
        channel_service.get_creator_id(id_user, req->id_channel);

    std::string responded_at = get_timestamp();

    invitation_service.reject_invitation(id_user, req->id_channel,
                                         responded_at);

    std::cout << "[INVITATION] User " << id_user
              << " -> declined the invitation " << req->id_channel << "\n";

    ServerSend::RejectInvitationResponse resp;
    resp.type = WizzMania::MessageType::INVITATION_REJECTED;
    resp.id_channel = req->id_channel;

    std::optional<ServerSend::Contact> rejecter_opt =
        user_service.get_contact(id_user);
    if (!rejecter_opt.has_value()) {
      throw WsError("User not found");
    }
    resp.contact = rejecter_opt.value();

    std::string resp_json = JsonHelpers::ServerSend::to_json(resp).dump();
    ws_manager.send_to_user(id_user, resp_json);
    ws_manager.send_to_user(id_creator, resp_json);

    std::string body =
        "User @" + std::to_string(id_user) + " rejected the chat!";

    message_controller.send_message_internal(1, req->id_channel, body,
                                             responded_at);
  } catch (const WsError& e) {
    std::cout << "an error occured " << e.get_message();
    return send_error(conn, "INTERNAL_ERROR", e.get_message());
  }
}

void InvitationController::broadcast_joined_notification(
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

void InvitationController::broadcast_invitation_notification(
    std::unordered_set<int64_t> participants,
    ServerSend::ChannelInvitation& invitation) {
  ws_manager.broadcast_to_users(
      participants, JsonHelpers::ServerSend::to_json(invitation).dump());
}