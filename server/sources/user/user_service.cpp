#include "user_service.hpp"

int64_t UserService::login(AuthMessages::LoginRequest login_request) {
  std::cout << "[LOGIN] Attempting login for user: " << login_request.username
            << "\n";
  int64_t id_user =
      db.verify_user(login_request.username, login_request.password);
  // TODO ADD EMAIL POSSIBILITY !!!!

  if (id_user < 0) {
    std::cout << "[LOGIN] Invalid credentials for: " << login_request.username
              << "\n";
    throw HttpError(401, "Invalid username or password");
  }
  return id_user;
}

bool UserService::has_access(int64_t id_user, int64_t id_channel) {
  bool is_system_user = (id_user == 1);

  bool has_channel_access = db.has_channel_access(id_user, id_channel);
  return (is_system_user || has_channel_access);
}

std::unordered_set<int64_t> UserService::get_users_by_channel(
    int64_t id_channel) {
  return db.get_channel_participants(id_channel);
}

std::optional<ServerSend::Contact> UserService::get_contact(int64_t id_user) {
  return db.get_contact(id_user);
}

int64_t UserService::get_id_user(const std::string& username) {
  std::optional<int64_t> id_user = db.get_id_user(username);
  if (!id_user.has_value()) {
    throw WsError("User " + username + " not found");
  }
  return id_user.value();
}

std::vector<ServerSend::Contact> UserService::get_contacts_from_channel(
    int64_t id_channel) {
  std::vector<ServerSend::Contact> contacts =
      db.get_channel_contacts(id_channel, ChannelStatus::PENDING);
  return contacts;
}

std::vector<ServerSend::Contact> UserService::get_all_user_contacts(
    int64_t id_user) {
  return db.get_user_contacts(id_user);
}