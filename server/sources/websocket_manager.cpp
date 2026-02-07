#include "websocket_manager.hpp"

void WebSocketManager::add_user(int64_t user_id, WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  this->user_sockets[user_id].insert(conn);
  this->socket_to_user[conn] = user_id;
  // lock guard will be released when going out of method
}

void WebSocketManager::remove_connection(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);

  auto it = this->socket_to_user.find(conn);
  if (it != this->socket_to_user.end()) {
    int64_t user_id = it->second;
    this->user_sockets[user_id].erase(conn);
    if (this->user_sockets[user_id].empty()) {
      this->user_sockets.erase(user_id);
    }
    this->socket_to_user.erase(conn);
  }
  // lock guard will be released when going out of method
}

bool WebSocketManager::is_authenticated(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  return this->socket_to_user.find(conn) != this->socket_to_user.end();
  // lock guard will be released when going out of method
}

std::optional<int64_t> WebSocketManager::get_user_id(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  auto it = this->socket_to_user.find(conn);
  if (it != this->socket_to_user.end()) {
    return it->second;
  }
  return std::nullopt;
}

// EVERYTHING USING VECTOR IS UTTERLY USELESS

// std::vector<WSConn> WebSocketManager::get_user_connections(int64_t user_id) {
//   std::lock_guard<std::mutex> lock(this->ws_mutex);

//   std::vector<WSConn> connections;
//   auto it = this->user_sockets.find(user_id);
//   if (it != this->user_sockets.end()) {
//     connections.assign(it->second.begin(), it->second.end());
//   }
//   return connections;
// }

// std::vector<WSConn> WebSocketManager::get_all_connections() {
//   std::lock_guard<std::mutex> lock(this->ws_mutex);
//   std::vector<WSConn> all_connections;
//   for (const auto& [conn, user_id] : this->socket_to_user) {
//     all_connections.push_back(conn);
//   }
//   return all_connections;
// }

void WebSocketManager::broadcast_to_all(const std::string& message) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  for (const auto& [conn, user_id] : this->socket_to_user) {
    conn->send_text(message);
  }
}

// V2 with type a bit less readable I think
// void WebSocketManager::broadcast_to_all(const std::string& message) {
//   std::lock_guard<std::mutex> lock(ws_mutex_);

//   for (const std::pair<WSConn, int64_t>& entry : socket_to_user_) {
//     WSConn conn = entry.first;
//     entry.first->send_text(message);
//   }
// }

void WebSocketManager::broadcast_to_users(const std::vector<int64_t>& user_ids,
                                          const std::string& message) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  for (int64_t user_id : user_ids) {
    auto it = this->user_sockets.find(user_id);
    if (it != this->user_sockets.end()) {
      for (WSConn conn : it->second) {
        conn->send_text(message);
      }
    }
  }
}

bool WebSocketManager::is_user_online(int64_t user_id) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  return user_sockets.find(user_id) != user_sockets.end();
}