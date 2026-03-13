#ifndef IWEBSOCKET_MANAGER_H
#define IWEBSOCKET_MANAGER_H

#include <crow.h>

#include <mutex>
#include <unordered_map>
#include <unordered_set>  // SHOULD APPEAR BEFORE CROW, ELSE EVERYTHING BREAKS !!!!!!
#include <vector>

#include "exception.hpp"

using WSConn = crow::websocket::connection*;

class IWebSocketManager {
 public:
  virtual ~IWebSocketManager() = default;

  virtual void add_user(int64_t id_user, WSConn conn,
                        const std::string& token) = 0;
  virtual void remove_connection(WSConn conn) = 0;
  virtual void disconnect_token(const std::string& token) = 0;
  virtual bool is_authenticated(WSConn conn) = 0;
  virtual std::optional<int64_t> get_id_user(WSConn conn) = 0;
  virtual std::vector<WSConn> get_user_connections(int64_t id_user) = 0;

  virtual void broadcast_to_all(const std::string& message) = 0;
  virtual void broadcast_to_users(const std::unordered_set<int64_t>& id_users,
                                  const std::string& message) = 0;
  virtual bool is_user_online(int64_t id_user) = 0;  // might not use this one
  virtual std::optional<int64_t> get_user_id(WSConn conn) = 0;
  virtual void send_to_user(int64_t id_user, const std::string& message) = 0;
  virtual void send_to_user_except(int64_t id_user, const std::string& message,
                                   const std::string& exclude_token) = 0;
  //   virtual void send_to_user_except_(int64_t id_user, const std::string&
  //   message,
  //                              const std::string& exclude_token) = 0;

  virtual std::string get_token_for_connection(WSConn conn) = 0;
  virtual void disconnect_user(int64_t id_user, const std::string& reason) = 0;
};

#endif