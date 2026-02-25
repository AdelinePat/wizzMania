#ifndef CHANNEL_CONTROLLER_H
#define CHANNEL_CONTROLLER_H

#include <crow.h>

#include <string>
#include <vector>

#include "channel_service.hpp"
#include "database.hpp"               // use
#include "exception.hpp"              // use
#include "helpers.hpp"                // use
#include "invitation_controller.hpp"  // use
#include "json_helpers.hpp"           // use
#include "message_structure.hpp"      // use
#include "optional"                   // use
#include "user_service.hpp"           // use
#include "utils.hpp"
#include "websocket_manager.hpp"  // use

class ChannelController {
 private:
  Database& db;
  WebSocketManager& ws_manager;
  //   InvitationService invitation_service;
  ChannelService channel_service;
  UserService user_service;
  InvitationController invitation_controller;

 public:
  explicit ChannelController(Database& db, WebSocketManager& ws)
      : db(db),
        ws_manager(ws),
        channel_service(db),
        user_service(db),
        invitation_controller(db, ws) {}

  // void create_channel(crow::websocket::connection& conn, int64_t id_user,
  //                     const crow::json::rvalue& json_msg);
  crow::response create_channel(int64_t id_user, const crow::request& req);

  crow::response leave_channel(int64_t id_user,
                               int64_t id_channel);
};

#endif