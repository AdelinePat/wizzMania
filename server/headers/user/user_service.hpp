#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

// #include "auth_controller.hpp"
#include "database.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"

class UserService {
  Database& db;

 private:
  // crow::response send_login_error(const int code, const std::string&
  // message); crow::response send_login_response(const int64_t id_user,
  //                                    const std::string& username,
  //                                    const std::string& token);

 public:
  explicit UserService(Database& db) : db(db) {}

  // crow::response login(Database& db, const crow::request& req);
  int64_t login(AuthMessages::LoginRequest login_request);

  bool has_access(int64_t id_user, int64_t id_channel);
  std::unordered_set<int64_t> get_users_by_channel(int64_t id_channel);
  std::optional<ServerSend::Contact> get_contact(int64_t id_user);
  int64_t get_id_user(const std::string& username);
  std::vector<ServerSend::Contact> get_contacts_from_channel(int64_t id_channel);
};

#endif