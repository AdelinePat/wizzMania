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
//   static const char* SECRET_KEY = Utils::get_env_var("SECRET_KEY");
// static std::string SECRET_KEY;

 public:
  //   explicit AuthController(Database& db, WebSocketManager& ws)
  //       : db(db), ws_manager(ws), auht_service(db) {}
  static std::string generateToken(int64_t id_user);

  static std::optional<int64_t> validateToken(const std::string& token);
};

#endif