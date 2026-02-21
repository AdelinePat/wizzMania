#include "channel_service.hpp"

// void get_channel() { resp.channel = db.get_channel(id_user, req->id_channel);
// }

// ==== CHANNEL INFO

// get entire channel info for a channel id
ServerSend::ChannelInfo ChannelService::get_channel(
    int64_t id_user, int64_t id_channel, ChannelStatus membership,
    ChannelStatus other_membership) {
  ServerSend::ChannelInfo channel =
      db.get_channel_info(id_user, id_channel, membership);
  channel.participants =
      db.get_participants(id_user, id_channel, membership, other_membership);
  channel.unread_count = db.get_unread_count(id_user, id_channel);
  channel.last_message = db.get_last_message(id_user, id_channel);
  channel.is_group = channel.participants.size() > 2;
  return channel;
}

int64_t ChannelService::get_creator_id(int64_t id_user, int64_t id_channel) {
  std::optional<int64_t> id_creator_opt = db.get_channel_creator(id_channel);

  if (!id_creator_opt.has_value() || id_creator_opt.value() == id_user) {
    throw WsError("Couldn't find creator of channel");
  }
  return id_creator_opt.value();
}

void ChannelService::generate_title(
    std::string& title, std::unordered_set<std::string>& usernames) {
  if (title.empty()) {
    for (const std::string& username : usernames) {
      title += title.empty() ? username : ", " + username;
    }
  }
  if (title.size() >= 60) {
    std::string truncatedTitle = title.substr(0, 55);
    title = truncatedTitle + "...";
  }
  //   return title;
}

int64_t ChannelService::create_channel(
    int64_t id_user, std::string& title, std::string& created_at,
    std::unordered_set<int64_t> participants) {
  std::optional<int64_t> id_channel_opt = db.create_channel_with_participants(
      id_user, title, created_at, participants);
  if (!id_channel_opt.has_value()) {
    std::cerr << "ERROR an error occured, channel couldn't be created";
    throw WsError(
        "[INTERNAL ERROR] An error occured, channel couldn't be created");
    // std::string error_msg = "Invalid username : " + username + " not
    // found"; this->send_error(conn, "ERROR",
    //                  "An error occured, channel couldn't be created");
    // return;
  }
  return id_channel_opt.value();
}