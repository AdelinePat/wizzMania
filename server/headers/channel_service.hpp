#ifndef CHANNEL_SERVICE_H
#define CHANNEL_SERVICE_H

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

class ChannelService {
  Database& db;

 private:
 public:
  explicit ChannelService(Database& db) : db(db) {}

  ServerSend::ChannelInfo get_channel(
      int64_t id_user, int64_t id_channel,
      ChannelStatus membership = ChannelStatus::ACCEPTED,
      ChannelStatus other_membership = ChannelStatus::ACCEPTED);

  int64_t get_creator_id(int64_t id_user, int64_t id_channel);

  void generate_title(std::string& title,
                      std::unordered_set<std::string>& usernames);

  int64_t create_channel(int64_t id_user, std::string& title,
                         std::string& created_at,
                         std::unordered_set<int64_t> participants);
};

#endif