#include "message_service.hpp"

int64_t MessageService::create_message(int64_t id_user, int64_t id_channel,
                                       std::string& body,
                                       std::string& timestamp) {
  std::optional<int64_t> id_message_opt =
      db.save_message(id_user, id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    throw WsError("[INIT] Error: message could not be save in db");
  }
  return id_message_opt.value();
}

// TODO CHECK NO HISTORY CONDITION!
std::vector<ServerSend::Message>
MessageService::get_messages_history_from_channel(int64_t id_channel,
                                                  int64_t before_id_message,
                                                  int limit) {
  std::vector<ServerSend::Message> messages =
      db.get_channel_history(id_channel, before_id_message, limit);
  if (messages.empty()) {
    std::cerr << "[HISTORY] : messages could not be retrieve in db\n";
    // actually possible to have no history, but I want to have a log about it
  }
  return messages;
}