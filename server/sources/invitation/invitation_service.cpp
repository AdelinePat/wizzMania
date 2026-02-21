#include "invitation_service.hpp"

void InvitationService::accept_invitation(int64_t id_user, int64_t id_channel,
                                          std::string& responded_at) {
  bool accepted = db.accept_invitation(id_user, id_channel, responded_at);
  if (!accepted) {
    throw WsError("[INVITATION ERROR] Couldn't accept the invitation");
  }
  return;
}

void InvitationService::reject_invitation(int64_t id_user, int64_t id_channel,
                                          std::string& responded_at) {
  bool rejected = db.reject_invitation(id_user, id_channel, responded_at);
  if (!rejected) {
    // this->send_error(conn, "[INVITATION ERROR]",
    //                  "Couldn't reject the invitation");
    // return;
    throw WsError("[INVITATION ERROR] Couldn't reject the invitation");
  }
}

std::vector<ServerSend::ChannelInvitation>
InvitationService::get_all_user_incoming_invitations(int64_t id_user) {
  // return db.get_incoming_invitations(id_user);
  std::lock_guard<std::mutex> lock(db.db_mutex);

  std::vector<ServerSend::ChannelInvitation> channels_invitations =
      db.get_invitations_base(id_user);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      db.get_participants_and_channel(id_user, ChannelStatus::PENDING,
                                      ChannelStatus::ACCEPTED);

  for (ServerSend::ChannelInvitation& channel : channels_invitations) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.other_participant_ids = it_participant->second;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }
  }

  return channels_invitations;
}

std::vector<ServerSend::ChannelInfo>
InvitationService::get_all_outgoing_invitations(int64_t id_user) {
  // return db.get_outgoing_invitations(id_user);
  std::lock_guard<std::mutex> lock(db.db_mutex);

  std::vector<ServerSend::ChannelInfo> channels_info =
      db.get_outgoing_invitations_base(id_user, ChannelStatus::ACCEPTED);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      db.get_participants_and_channel(id_user, ChannelStatus::ACCEPTED,
                                      ChannelStatus::PENDING);

  for (ServerSend::ChannelInfo& channel : channels_info) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.participants = it_participant->second;
      channel.is_group = channel.participants.size() > 2;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }
  }
  return channels_info;
}