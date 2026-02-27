#include "invitation_controller.hpp"

crow::response InvitationController::accept_invitation(int64_t id_user,
                                                       int64_t id_channel,
                                                       std::string& token) {
  if (!user_service.has_pending_invitation(id_user, id_channel)) {
    throw ForbiddenError("No pending invitation for this channel");
  }
  std::cout << "[INVITATION] User " << id_user << " -> accepts to Channel "
            << id_channel << "\n";

  std::string responded_at = Utils::get_timestamp();
  // try {
  invitation_service.accept_invitation(id_user, id_channel, responded_at);

  ServerSend::AcceptInvitationResponse resp;
  resp.type = WizzMania::MessageType::INVITATION_ACCEPTED;

  resp.channel = channel_service.get_channel(id_user, id_channel);

  std::string channel_info_str =
      JsonHelpers::ServerSendHelpers::to_json(resp).dump();

  // send new participant list to all current participant of channel
  broadcast_joined_notification(id_user, id_channel, resp.channel.participants);

  // need to let all the device of the user that the invitation was accepted and
  // remove it from "incoming invitation"
  // ws_manager.send_to_user(id_user, channel_info_str);

  std::string body = "User @" + std::to_string(id_user) + " joined the chat!";
  message_controller.send_message_internal(1, id_channel, body, responded_at);

  return send_accept_invitation(id_user, channel_info_str, token);
}

crow::response InvitationController::cancel_invitation(int64_t id_user,
                                                       int64_t id_channel,
                                                       std::string& token) {
  if (channel_service.get_creator_id(id_channel) != id_user) {
    throw ForbiddenError("Can't cancel an invitation you didn't create");
  }
  if (!user_service.has_access(id_user, id_channel)) {
    throw ForbiddenError("Can't access this channel");
  }

  int64_t total_accepted_participant =
      channel_service.get_number_accepted_users_in_channel(id_channel);

  // Creator counts as 1 accepted; > 1 means another user has already accepted
  if (total_accepted_participant > 1) {
    throw ForbiddenError(
        "Can't cancel an invitation that was already accepted");
  }
  // need to get all pending user before channel deletion
  std::unordered_set<int64_t> pending_users =
      user_service.get_pending_users_by_channel(id_channel);
  std::string responded_at = Utils::get_timestamp();
  invitation_service.cancel_invitation(id_user, id_channel, responded_at);

  if (channel_service.does_channel_exist(id_channel)) {
    throw ConflictError(
        "Channel should have been deleted if creator cancel invitation");
  }

  std::cout << "[INVITATION] User " << id_user << " -> canceled the invitation "
            << id_channel << "\n";

  // Using the same structure as RejectInvitation, send the same data!
  ServerSend::RejectInvitationResponse resp;
  resp.type = WizzMania::MessageType::CANCEL_INVITATION;
  resp.id_channel = id_channel;

  std::optional<ServerSend::Contact> canceler =
      user_service.get_contact(id_user);
  if (!canceler.has_value()) {
    throw NotFoundError("User not found");
  }
  resp.contact = canceler.value();

  // send rejection to creator !
  std::string resp_json = JsonHelpers::ServerSendHelpers::to_json(resp).dump();
  ws_manager.broadcast_to_users(pending_users, resp_json);
  ws_manager.send_to_user_except(id_user, resp_json, token);

  return crow::response(200, resp_json);
}

crow::response InvitationController::reject_invitation(int64_t id_user,
                                                       int64_t id_channel,
                                                       std::string& token) {
  if (!user_service.has_pending_invitation(id_user, id_channel)) {
    throw ForbiddenError("No pending invitation for this channel");
  }
  try {
    std::string responded_at = Utils::get_timestamp();

    int64_t id_creator = channel_service.get_inviter_id(id_user, id_channel);

    invitation_service.reject_invitation(id_user, id_channel, responded_at);

    bool channel_exists = channel_service.does_channel_exist(id_channel);

    std::cout << "[INVITATION] User " << id_user
              << " -> declined the invitation " << id_channel << "\n";

    ServerSend::RejectInvitationResponse resp;
    resp.type = WizzMania::MessageType::INVITATION_REJECTED;
    resp.id_channel = id_channel;

    std::optional<ServerSend::Contact> rejecter_opt =
        user_service.get_contact(id_user);
    if (!rejecter_opt.has_value()) {
      throw NotFoundError("User not found");
    }
    resp.contact = rejecter_opt.value();

    // send rejection to creator !
    std::string resp_json =
        JsonHelpers::ServerSendHelpers::to_json(resp).dump();
    ws_manager.send_to_user(id_creator, resp_json);
    ws_manager.send_to_user_except(id_user, resp_json, token);

    if (channel_exists) {
      std::string body =
          "User @" + std::to_string(id_user) + " rejected the chat!";

      message_controller.send_message_internal(1, id_channel, body,
                                               responded_at);
    }
    return crow::response(200, resp_json);
  } catch (const WizzManiaError& e) {
    return WizzManiaError::send_http_error(e.get_code(), e.get_message());
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
      JsonHelpers::ServerSendHelpers::to_json(joined_notification).dump();
  std::unordered_set<int64_t> participants_set =
      db.get_channel_participants(id_channel);

  participants_set.erase(id_user);
  ws_manager.broadcast_to_users(participants_set, joined_json);
}

void InvitationController::broadcast_invitation_notification(
    std::unordered_set<int64_t> participants,
    ServerSend::ChannelInvitation& invitation) {
  participants.erase(invitation.id_inviter);
  ws_manager.broadcast_to_users(
      participants, JsonHelpers::ServerSendHelpers::to_json(invitation).dump());
}

crow::response InvitationController::send_accept_invitation(
    int64_t id_user, const std::string& channel_info_str,
    const std::string& token) {
  // Only send WS to OTHER devices of this user, not the one that made the HTTP
  // request
  ws_manager.send_to_user_except(id_user, channel_info_str, token);

  return crow::response(200, channel_info_str);
}