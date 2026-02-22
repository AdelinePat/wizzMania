#include "helpers.hpp"

ServerSend::Message Structure::create_message_struct(int64_t id_message, int64_t id_user,
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

ServerSend::ChannelInvitation Structure::create_invitation_struct(
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

ServerSend::ChannelInfo Structure::create_empty_channel_info_struct(
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

