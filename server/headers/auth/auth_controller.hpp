#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <crow.h>

#include <string>
#include <vector>

#include "auth_service.hpp"
#include "database.hpp"           // use
#include "exception.hpp"          // use
#include "helpers.hpp"            // use
#include "json_helpers.hpp"       // use
#include "message_structure.hpp"  // use
#include "optional"               // use
#include "utils.hpp"
#include "websocket_manager.hpp"  // use

class AuthController {
 private:
  WebSocketManager& ws_manager;
  AuthService auth_service;

 public:
  explicit AuthController(WebSocketManager& ws)
      : ws_manager(ws), auth_service() {}
  std::string generateToken(int64_t id_user);

//   int64_t validate_token(const std::string& token);

  void authenticate_ws(crow::websocket::connection& conn,
                       const crow::json::rvalue& json_msg);

  void auth_success(crow::websocket::connection& conn,
                    const int64_t validated_id_user);
  void auth_error(crow::websocket::connection& conn,
                  const std::string& message);
};

#endif