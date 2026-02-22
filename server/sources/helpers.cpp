#include "helpers.hpp"

ServerSend::Message create_message_struct(int64_t id_message, int64_t id_user,
                                          const std::string& body,
                                          const std::string& timestamp) {
  ServerSend::Message message;
  message.id_message = id_message;
  message.id_sender = id_user;
  message.body = body;
  message.timestamp = timestamp;
  message.is_system = id_user == 1;
  return message;
}

ServerSend::ChannelInvitation create_invitation_struct(
    int64_t id_channel, int64_t id_inviter,
    const std::vector<ServerSend::Contact>& other_participants,
    std::string& title) {
  ServerSend::ChannelInvitation invitation;
  invitation.type = WizzMania::MessageType::CHANNEL_INVITATION;
  invitation.id_channel = id_channel;
  invitation.id_inviter = id_inviter;
  invitation.other_participant_ids = other_participants;
  invitation.title = title;
  return invitation;
}

ServerSend::ChannelInfo create_empty_channel_info_struct(
    int64_t id_channel, int64_t created_by,
    const std::vector<ServerSend::Contact>& other_participants,
    std::string& title) {
  ServerSend::ChannelInfo info;
  info.id_channel = id_channel;
  info.title = title;
  info.participants = other_participants;
  info.is_group = (info.participants.size() + 1) > 2;
  info.created_by = created_by;
  return info;
}

// void send_error(crow::websocket::connection& conn,
//                 const std::string& error_code,
//                 const std::string& error_message) {
//   ServerSend::ErrorResponse err;
//   err.type = WizzMania::MessageType::ERROR;
//   err.error_code = error_code;
//   err.message = error_message;

//   conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
// }

// CHANGES!

// void send_ws_error(crow::websocket::connection& conn, WizzManiaError error) {
//   ServerSend::ErrorResponse err;
//   err.type = WizzMania::MessageType::ERROR;
//   err.error_code = error.get_code();
//   err.message = error.get_message();

//   conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
// }

// crow::response send_http_error(int code, const std::string& message) {
//   crow::json::wvalue body;
//   body["error"] = message;
//   auto res = crow::response(code, body.dump());
//   res.add_header("Content-Type", "application/json");
//   return res;
// }