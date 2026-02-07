#ifndef WEBSOCKET_MANAGER_H
#define WEBSOCKET_MANAGER_H

#include <crow.h>

#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using WSConn = crow::websocket::connection*;

class WebSocketManager {
 private:
  // --------------------
  // WebSocket connection maps
  // --------------------
  std::unordered_map<int64_t, std::unordered_set<WSConn>> user_sockets;
  std::unordered_map<WSConn, int64_t> socket_to_user;
  std::mutex ws_mutex;

 public:
  void add_user(int64_t user_id, WSConn conn);
  void remove_connection(WSConn conn);
  bool is_authenticated(WSConn conn);
  std::optional<int64_t> get_user_id(WSConn conn);
  std::vector<WSConn> get_user_connections(int64_t user_id);
  // Does it have to be a vector ? Isn't a queue or stack
  // enough since I'll never access it using an index?
  // std::vector<WSConn> get_all_connections();
  void broadcast_to_all(const std::string& message);
  void broadcast_to_users(const std::vector<int64_t>& user_ids,
                          const std::string& message);

  bool is_user_online(int64_t user_id);  // might not use this one
  // TODO: Implement
};

#endif