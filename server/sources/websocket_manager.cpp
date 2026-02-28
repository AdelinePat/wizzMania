#include "websocket_manager.hpp"

void WebSocketManager::add_user(int64_t id_user, WSConn conn,
                                const std::string& token) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  this->user_sockets[id_user].insert(conn);
  this->socket_to_user[conn] = id_user;
  this->token_to_socket[token] = conn;
  this->socket_to_token[conn] = token;
  // lock guard will be released when going out of method
}

void WebSocketManager::remove_connection(WSConn conn) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  auto it_user = this->socket_to_user.find(conn);
  if (it_user != this->socket_to_user.end()) {
    int64_t id_user = it_user->second;
    this->user_sockets[id_user].erase(conn);
    if (this->user_sockets[id_user].empty()) {
      this->user_sockets.erase(id_user);
    }
    this->socket_to_user.erase(conn);
  }
  auto it_token = this->socket_to_token.find(conn);
  if (it_token != this->socket_to_token.end()) {
    this->token_to_socket.erase(it_token->second);
    this->socket_to_token.erase(it_token);
  }
  // lock guard will be released when going out of method
}

void WebSocketManager::disconnect_token(const std::string& token) {
  std::lock_guard<std::mutex> lock(this->ws_mutex);
  auto it = this->token_to_socket.find(token);
  if (it == this->token_to_socket.end()) return;

  WSConn conn = it->second;
  conn->close("logout");
  // cleanup is handled by onclose → remove_connection
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

void WebSocketManager::broadcast_to_users(
    const std::unordered_set<int64_t>& id_users, const std::string& message) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  for (int64_t id_user : id_users) {
    this->send_to_user_(id_user, message);
  }
}

// Private method that sends message to all user's connection, no guard lock
void WebSocketManager::send_to_user_(int64_t id_user,
                                     const std::string& message) {
  // get all connections for this user, send to each
  auto it = this->user_sockets.find(id_user);
  if (it != this->user_sockets.end()) {
    for (WSConn conn : it->second) {
      conn->send_text(message);
    }
  }
}

void WebSocketManager::send_to_user_except_(int64_t id_user,
                                     const std::string& message, const std::string& exclude_token) {
  // get all connections for this user, send to each
   auto it = user_sockets.find(id_user);
  if (it == user_sockets.end()) return;

  for (WSConn conn : it->second) {
    auto token_it = socket_to_token.find(conn);
    if (token_it != socket_to_token.end() && token_it->second != exclude_token) {
      conn->send_text(message);
    }
  }
}

// Public method that sends message to all user's connection, guard lock !
void WebSocketManager::send_to_user(int64_t id_user,
                                    const std::string& message) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  auto it = this->user_sockets.find(id_user);
  if (it != this->user_sockets.end()) {
    for (WSConn conn : it->second) {
      conn->send_text(message);
    }
  }
}


void WebSocketManager::send_to_user_except(int64_t id_user,
                                           const std::string& message,
                                           const std::string& exclude_token) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  send_to_user_except_(id_user, message, exclude_token);
  // auto it = user_sockets.find(id_user);
  // if (it == user_sockets.end()) return;

  // for (WSConn conn : it->second) {
  //   auto token_it = socket_to_token.find(conn);
  //   if (token_it != socket_to_token.end() && token_it->second != exclude_token) {
  //     conn->send_text(message);
  //   }
  // }
}


bool WebSocketManager::is_user_online(int64_t id_user) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  return user_sockets.find(id_user) != user_sockets.end();
}

std::optional<int64_t> WebSocketManager::get_user_id(WSConn conn) {
  auto it = socket_to_user.find(conn);
  if (it != socket_to_user.end()) {
    return std::optional<int64_t>(it->second);
  } else {
    return std::nullopt;
  }
}

std::vector<WSConn> WebSocketManager::get_user_connections(int64_t id_user) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  std::vector<WSConn> connections;
  std::unordered_map<int64_t, std::unordered_set<WSConn>>::iterator it =
      user_sockets.find(id_user);
  if (it != user_sockets.end()) {
    for (WSConn conn : it->second) {
      connections.push_back(conn);
    }
  }
  return connections;
}

std::string WebSocketManager::get_token_for_connection(WSConn conn) {
  std::lock_guard<std::mutex> lock(ws_mutex);
  auto it = socket_to_token.find(conn);
  if (it != socket_to_token.end()) {
    return it->second;
  }
  return "";
}