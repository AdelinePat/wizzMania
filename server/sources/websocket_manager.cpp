#include "websocket_manager.hpp"

void WebSocketManager::add_user(int64_t id_user, WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  this->user_sockets[id_user].insert(conn);
  this->socket_to_user[conn] = id_user;
  // lock guard will be released when going out of method
}

void WebSocketManager::remove_connection(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);

  auto it = this->socket_to_user.find(conn);
  if (it != this->socket_to_user.end()) {
    int64_t id_user = it->second;
    this->user_sockets[id_user].erase(conn);
    if (this->user_sockets[id_user].empty()) {
      this->user_sockets.erase(id_user);
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

std::optional<int64_t> WebSocketManager::get_id_user(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  auto it = this->socket_to_user.find(conn);
  if (it != this->socket_to_user.end()) {
    return it->second;
  }
  return std::nullopt;
}

void WebSocketManager::broadcast_to_all(const std::string& message) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  for (const auto& [conn, id_user] : this->socket_to_user) {
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

void WebSocketManager::broadcast_to_users(const std::vector<int64_t>& id_users,
                                          const std::string& message) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  for (int64_t id_user : id_users) {
    auto it = this->user_sockets.find(id_user);
    if (it != this->user_sockets.end()) {
      for (WSConn conn : it->second) {
        conn->send_text(message);
      }
    }
  }
}

bool WebSocketManager::is_user_online(int64_t id_user) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  return user_sockets.find(id_user) != user_sockets.end();
}

int64_t WebSocketManager::get_user_id(WSConn conn) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  // int64_t id_user = this->.socket_to_user.at(conn);
  // return id_user ? id_user : -1;
  auto it = socket_to_user.find(conn);
  return it != socket_to_user.end() ? it->second : -1;
}