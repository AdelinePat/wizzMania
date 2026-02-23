#include "message_service.hpp"
#include <vector>

int64_t MessageService::create_message(int64_t id_user, int64_t id_channel,
                                       std::string& body,
                                       std::string& timestamp) {
  std::optional<int64_t> id_message_opt =
      db.save_message(id_user, id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    throw InternalError("Message could not be saved in DB");
  }
  return id_message_opt.value();
}

//===MARK AS READ===//
bool MessageService::mark_as_read(int64_t id_user, int64_t id_channel, int64_t last_id_message) {
  bool updated = db.update_last_read_message(id_user, id_channel, last_id_message);
  if (!updated) {
    std::cerr << "[MARK_AS_READ] Could not update last_read_id_message for user "
              << id_user << " in channel " << id_channel << "\n";
  }
  return updated;
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
  }
  return messages;
}