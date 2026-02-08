#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <crow.h>

#include <mutex>
#include <vector>

#include "database.hpp"
#include "json_helpers.hpp"
#include "auth.hpp"
#include "message_structure.hpp"
// using WSConn = crow::websocket::connection*;

class HttpManager {
 private:
 public:
  crow::response login(Database& db, const crow::request& req);
};

#endif