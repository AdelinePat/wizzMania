#include "message_service.hpp"

int64_t MessageService::create_message(int64_t id_user, int64_t id_channel,
                                  std::string& body, std::string& timestamp) {
  std::optional<int64_t> id_message_opt =
      db.save_message(id_user, id_channel, body, timestamp);
  if (!id_message_opt.has_value()) {
    std::cerr << "[INIT] Error: message could not be save in db\n";
    // return ;
    throw WsError("[INIT] Error: message could not be save in db");
  }
  return id_message_opt.value();
}