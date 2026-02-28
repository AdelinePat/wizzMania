#include "channel_service.hpp"

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
  // channel.is_group = channel.participants.size() > 2;
  channel.is_group = db.get_number_invited_users_in_channel(id_channel) > 2;
  return channel;
}

int64_t ChannelService::get_inviter_id(int64_t id_user, int64_t id_channel) {
  // std::optional<int64_t> id_creator_opt = db.get_channel_creator(id_channel);
  int64_t id_creator = this->get_creator_id(id_channel);
  if (!id_creator || id_creator == id_user) {
    throw NotFoundError("Couldn't find inviter for this channel");
  }
  return id_creator;
}

int64_t ChannelService::get_creator_id(int64_t id_channel) {
  std::optional<int64_t> id_creator_opt = db.get_channel_creator(id_channel);
  if (!id_creator_opt.has_value()) {
    throw NotFoundError("Couldn't find creator for this channel");
  }
  return id_creator_opt.value();
}

void ChannelService::generate_title(std::string& title,
                                    std::unordered_set<std::string>& usernames,
                                    int64_t id_creator) {
  if (title.empty()) {
    // add creator name to title!
    title += db.get_contact(id_creator).value().username;

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
    throw InternalError("An error occurred, channel couldn't be created");
    // throw WsError(
    // "[INTERNAL ERROR] An error occured, channel couldn't be created");
    // std::string error_msg = "Invalid username : " + username + " not
    // found"; this->send_error(conn, "ERROR",
    //                  "An error occured, channel couldn't be created");
    // return;
  }
  return id_channel_opt.value();
}

std::vector<ServerSend::ChannelInfo> ChannelService::get_all_user_channels(
    int64_t id_user) {
  // return db.get_initial_channels(id_user);

  // Create ChannelInfo list for initial_data
  // std::vector<ServerSend::ChannelInfo> Database::get_initial_channels(
  //     const int64_t id_user) {
  std::lock_guard<std::mutex> lock(db.db_mutex);

  std::vector<ServerSend::ChannelInfo> channels_info = db.get_channels(id_user);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      db.get_participants_and_channel(id_user);
  std::map<int64_t, int64_t> channels_unread_count =
      db.get_unread_count(id_user);
  std::map<int64_t, ServerSend::Message> channels_last_message =
      db.get_last_messages(id_user);

  for (ServerSend::ChannelInfo& channel : channels_info) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.participants = it_participant->second;
      channel.is_group = channel.participants.size() > 2;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }

    auto it_unread_count = channels_unread_count.find(channel.id_channel);
    if (it_unread_count != channels_unread_count.end()) {
      channel.unread_count = it_unread_count->second;
    } else {
      channel.unread_count = 0;
    }

    auto it_last_message = channels_last_message.find(channel.id_channel);
    if (it_last_message != channels_last_message.end()) {
      channel.last_message = it_last_message->second;
    } else {
      std::cerr << "[DB] Warning: no last message found for channel "
                << channel.id_channel << "\n";
    }
  }
  return channels_info;
}

void ChannelService::leave_channel(int64_t id_user, int64_t id_channel) {
  db.leave_channel(id_user, id_channel);
}

std::optional<int64_t> ChannelService::find_existing_channel(
    const std::unordered_set<int64_t>& invited_participants) {
  return db.find_existing_channel(invited_participants);
}

bool ChannelService::does_channel_exist(int64_t id_channel) {
    std::optional<bool> result = db.does_channel_exist(id_channel);
  if (!result.has_value()) {
    throw InternalError("Could not check channel existence");
  }
  return result.value();
}