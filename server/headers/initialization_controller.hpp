#ifndef INITIALIZATION_CONTROLLER_H
#define INITIALIZATION_CONTROLLER_H

#include <crow.h>

#include <string>
#include <vector>

#include "auth_controller.hpp"
#include "channel_service.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "messages.hpp"
#include "optional"
#include "user_service.hpp"
#include "websocket_manager.hpp"
#include "invitation_service.hpp"

class InitializationController {
 private:
  Database& db;
  WebSocketManager& ws;
  ChannelService channel_service;
  UserService user_service;
  InvitationService invitation_service;
  //   UserService user_service;
  //   AuthController auth_controller;
  ServerSend::InitialDataResponse get_initial_data(const int64_t id_user);

 public:
  explicit InitializationController(Database& db, WebSocketManager& ws)
      : db(db), ws(ws), channel_service(db), user_service(db), invitation_service(db) {}
  void initial_data(crow::websocket::connection& conn, int64_t id_user);
};

#endif