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
    throw UnauthorizedError("Invalid username or password");
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

std::unordered_set<int64_t> UserService::get_pending_users_by_channel(
    int64_t id_channel) {
  return db.get_channel_participants(id_channel, ChannelStatus::PENDING);
}

std::optional<ServerSend::Contact> UserService::get_contact(int64_t id_user) {
  return db.get_contact(id_user);
}

int64_t UserService::get_id_user(const std::string& username) {
  std::optional<int64_t> id_user = db.get_id_user(username);
  if (!id_user.has_value()) {
    throw NotFoundError("User " + username + " not found");
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

bool UserService::has_pending_invitation(int64_t id_user, int64_t id_channel) {
  return db.has_channel_access(id_user, id_channel, ChannelStatus::PENDING);
}

int64_t UserService::register_user(const std::string& username,
                                   const std::string& email,
                                   const std::string& password) {
  // clean and validate username
  std::string clean_name =
      Utils::clean_username(username);  // throws badInput if invalid char
  // clean username already calls trim() inside

  // validate email format
  if (!Utils::is_valid_email(email)) {
    throw BadRequestError("Invalid email format");
  }

  // validate password
  if (!Utils::is_valid_password(password)) {
    throw BadRequestError(
        "Password must be at least 8 characters with uppercase, lowercase and "
        "special character");
  }

  // check if username already taken (reuse existing get_id_user)
  std::optional<int64_t> existing_user = db.get_id_user(clean_name);
  if (existing_user.has_value()) {
    throw ConflictError("Username already taken");
  }

  // check if email is already taken
  if (db.email_exists(email)) {
    throw ConflictError("Email already in use");
  }

  // create user in Database
  std::optional<int64_t> new_id = db.create_user(clean_name, email, password);
  if (!new_id.has_value()) {
    throw InternalError("Failed to create user");
  }
  return new_id.value();
}


// DELETE USER

void UserService::delete_user(int64_t id_user) {
  // verify user exists
  std::optional<ServerSend::Contact> user = db.get_contact(id_user);
  if(!user.has_value()) {
    throw NotFoundError("User not found");
  }

  //Delete the user in the DB
  bool deleted = db.delete_user(id_user);
  if(!deleted) {
    throw InternalError("Failed to delete user");
  }
}