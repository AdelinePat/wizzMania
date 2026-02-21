#ifndef MESSAGE_SERVICE_H
#define MESSAGE_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "auth.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
// using WSConn = crow::websocket::connection*;

class MessageService {
  Database& db;

 private:
 public:
  explicit MessageService(Database& db) : db(db) {}

  int64_t create_message(int64_t id_user, int64_t id_channel, std::string& body,
                         std::string& timestamp);
  int64_t get_creator_id(int64_t id_user, int64_t id_channel);
};

#endif