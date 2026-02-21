#include "invitation_service.hpp"

void InvitationService::accept_invitation(int64_t id_user, int64_t id_channel,
                                          std::string& responded_at) {
  bool accepted = db.accept_invitation(id_user, id_channel, responded_at);
  if (!accepted) {
    throw WsError("[INVITATION ERROR] Couldn't accept the invitation");
  }
  return;
}

void InvitationService::reject_invitation(int64_t id_user, int64_t id_channel,
                                          std::string& responded_at) {
  bool rejected = db.reject_invitation(id_user, id_channel, responded_at);
  if (!rejected) {
    // this->send_error(conn, "[INVITATION ERROR]",
    //                  "Couldn't reject the invitation");
    // return;
    throw WsError("[INVITATION ERROR] Couldn't reject the invitation");
  }
}