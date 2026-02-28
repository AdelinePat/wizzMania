#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <crow.h>

#include <mutex>
#include <unordered_map>
#include <unordered_set>  // SHOULD APPEAR BEFORE CROW, ELSE EVERYTHING BREAKS !!!!!!
#include <vector>

#include "exception.hpp"

using WSConn = crow::websocket::connection*;

class WebSocketManager {
 private:
  // --------------------
  // WebSocket connection maps
  // --------------------
  std::unordered_map<int64_t, std::unordered_set<WSConn>> user_sockets;
  std::unordered_map<WSConn, int64_t> socket_to_user;
  std::unordered_map<std::string, WSConn> token_to_socket;
  std::unordered_map<WSConn, std::string> socket_to_token;

  std::mutex ws_mutex;
  void send_to_user_(int64_t id_user, const std::string& message);

 public:
  void add_user(int64_t id_user, WSConn conn, const std::string& token);
  void remove_connection(WSConn conn);
  void disconnect_token(const std::string& token);
  bool is_authenticated(WSConn conn);
  std::optional<int64_t> get_id_user(WSConn conn);
  std::vector<WSConn> get_user_connections(int64_t id_user);

  void broadcast_to_all(const std::string& message);
  void broadcast_to_users(const std::unordered_set<int64_t>& id_users,
                          const std::string& message);

  bool is_user_online(int64_t id_user);  // might not use this one
  std::optional<int64_t> get_user_id(WSConn conn);
  void send_to_user(int64_t id_user, const std::string& message);
  void send_to_user_except(int64_t id_user, const std::string& message,
                           const std::string& exclude_token);
  void send_to_user_except_(int64_t id_user, const std::string& message,
                            const std::string& exclude_token);

  std::string get_token_for_connection(WSConn conn);
};

#endif