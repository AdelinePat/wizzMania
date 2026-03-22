#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <crow.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

// #include "auth_controller.hpp"
#include <unordered_map>
#include <unordered_set>

#include "database.hpp"
#include "idatabase.hpp"
#include "exception.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "utils.hpp"

class UserService {
  IDatabase& db;

 private:
  // crow::response send_login_error(const int code, const std::string&
  // message); crow::response send_login_response(const int64_t id_user,
  //                                    const std::string& username,
  //                                    const std::string& token);

 public:
  explicit UserService(IDatabase& db) : db(db) {}

  // crow::response login(Database& db, const crow::request& req);
  int64_t login(AuthMessages::LoginRequest login_request);

  bool has_access(int64_t id_user, int64_t id_channel);
  std::unordered_set<int64_t> get_users_by_channel(int64_t id_channel);
  std::unordered_set<int64_t> get_pending_users_by_channel(int64_t id_channel);
  std::optional<ServerSend::Contact> get_contact(int64_t id_user);
  int64_t get_id_user(const std::string& username);
  std::vector<ServerSend::Contact> get_contacts_from_channel(
      int64_t id_channel);
  std::vector<ServerSend::Contact> get_all_user_contacts(int64_t id_user);
  bool has_pending_invitation(int64_t id_user, int64_t id_channel);

  int64_t register_user(const std::string& username, const std::string& email,
                        const std::string& password);

  void delete_user(int64_t id_user,
                   std::unordered_map<int64_t, std::unordered_set<int64_t>>&
                       deleted_channels,
                   std::unordered_map<int64_t, std::unordered_set<int64_t>>&
                       canceled_invitations);
};

#endif