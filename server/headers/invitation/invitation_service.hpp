#ifndef INVITATION_SERVICE_H
#define INVITATION_SERVICE_H

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

class InvitationService {
  Database& db;

 private:
 public:
  explicit InvitationService(Database& db) : db(db) {}
  void accept_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at);
  void reject_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at);
};

#endif