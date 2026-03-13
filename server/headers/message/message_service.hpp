#ifndef MESSAGE_SERVICE_H
#define MESSAGE_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "database.hpp"
#include "idatabase.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"

class MessageService {
  IDatabase& db;

 public:
  explicit MessageService(IDatabase& db) : db(db) {}

  int64_t create_message(int64_t id_user, int64_t id_channel, std::string& body,
                         std::string& timestamp);
  int64_t get_creator_id(int64_t id_user, int64_t id_channel);
  std::vector<ServerSend::Message> get_messages_history_from_channel(
      int64_t id_channel, int64_t before_id_message, int limit);
  bool mark_as_read(int64_t id_user, int64_t id_channel, int64_t last_id_message);
};

#endif