#ifndef INVITATION_CONTROLLER_H
#define INVITATION_CONTROLLER_H

#include <crow.h>

#include <string>
#include <vector>

#include "channel_service.hpp"
#include "database.hpp"            // use
#include "exception.hpp"           // use
#include "helpers.hpp"             // use
#include "invitation_service.hpp"  // use
#include "json_helpers.hpp"        // use
#include "message_controller.hpp"
#include "message_structure.hpp"  // use
#include "optional"               // use
#include "user_service.hpp"       // use
#include "user_service.hpp"
#include "utils.hpp"
#include "websocket_manager.hpp"  // use

class InvitationController {
 private:
  Database& db;
  WebSocketManager& ws_manager;
  InvitationService invitation_service;
  ChannelService channel_service;
  MessageController message_controller;
  UserService user_service;

  void broadcast_joined_notification(
      const int64_t id_user, const int64_t id_channel,
      std::vector<ServerSend::Contact>& participants);

 public:
  explicit InvitationController(Database& db, WebSocketManager& ws)
      : db(db),
        ws_manager(ws),
        invitation_service(db),
        channel_service(db),
        message_controller(db, ws),
        user_service(db) {}

  void accept_invitation(crow::websocket::connection& conn, int64_t id_user,
                         const crow::json::rvalue& json_msg);
  void reject_invitation(crow::websocket::connection& conn, int64_t id_user,
                         const crow::json::rvalue& json_msg);
  void broadcast_invitation_notification(
      std::unordered_set<int64_t> participants,
      ServerSend::ChannelInvitation& invitation);
};

#endif