#ifndef INVITATION_SERVICE_H
#define INVITATION_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "database.hpp"
#include "idatabase.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "idatabase.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"

class InvitationService {
  IDatabase& db;

 private:
 public:
  explicit InvitationService(IDatabase& db) : db(db) {}

  void accept_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at);
  void reject_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at);

  std::vector<ServerSend::ChannelInvitation> get_all_user_incoming_invitations(
      int64_t id_user);

  std::vector<ServerSend::ChannelInfo> get_all_outgoing_invitations(
      int64_t id_user);

  void cancel_invitation(int64_t id_user, int64_t id_channel,
                         std::string& responded_at);
};

#endif