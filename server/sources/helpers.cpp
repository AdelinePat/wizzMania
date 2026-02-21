#include "helpers.hpp"

uint16_t get_server_port() {
  const char* portStr = std::getenv("SERVER_PORT");
  uint16_t port = 8888;

  if (!portStr) {
    std::cout << "[WARN] SERVER_PORT not set. Using default 8888\n";
    portStr = "8888";
  }

  try {
    int temp = std::stoi(portStr);
    if (temp > 0 && temp <= 65535)
      port = static_cast<uint16_t>(temp);
    else
      std::cerr << "[WARN] SERVER_PORT out of range, using default\n";
  } catch (...) {
    std::cerr << "[WARN] Invalid SERVER_PORT, using default\n";
  }

  return port;
}

std::string get_timestamp() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&now_time), "%Y-%m-%dT%H:%M:%S") << "Z";
  std::string timestamp = oss.str();
  return timestamp;
}

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
  info.is_group = info.participants.size() > 2;
  info.created_by = created_by;
  return info;
}

void send_error(crow::websocket::connection& conn,
                const std::string& error_code,
                const std::string& error_message) {
  ServerSend::ErrorResponse err;
  err.type = WizzMania::MessageType::ERROR;
  err.error_code = error_code;
  err.message = error_message;

  conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
}
