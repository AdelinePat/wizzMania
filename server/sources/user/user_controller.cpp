
#include "user_controller.hpp"

crow::response UserController::login(const crow::request& req) {
  std::cout << "[LOGIN] Received login request\n";

  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    std::cout << "[LOGIN] Invalid JSON\n";
    throw BadRequestError("Invalid JSON");
    // return this->send_login_error(error);
  }

  std::optional<AuthMessages::LoginRequest> login_req =
      JsonHelpers::Auth::parse_login_request(json_body);
  if (!login_req.has_value()) {
    std::cout << "[LOGIN] Missing required fields\n";
    throw BadRequestError("Missing username or password");
    // return this->send_login_error(error);
  }

  int64_t id_user = this->user_service.login(login_req.value());
  if (id_user == 1) {
    throw UnauthorizedError("Invalid username or password");
  };
  std::string token = auth_controller.generateToken(id_user);
  return this->send_login_response(id_user, login_req->username, token);
}

crow::response UserController::logout(const std::string& token) {
  std::cout << "[LOGOUT] Received logout request\n";
  ws.disconnect_token(token);
  return crow::response(200);
  // }
}

// Change into standard http response for error
crow::response UserController::send_login_error(const WizzManiaError& e) {
  AuthMessages::LoginResponse error_resp;
  error_resp.success = false;
  error_resp.message = e.get_message();
  return crow::response(e.get_code(), JsonHelpers::Auth::to_json(error_resp));
}

crow::response UserController::send_login_response(const int64_t id_user,
                                                   const std::string& username,
                                                   const std::string& token) {
  std::cout << "[LOGIN] Login successful! User ID: " << id_user << "\n";
  std::cout << "[LOGIN] Token generated: " << token.substr(0, 20) << "...\n";

  AuthMessages::LoginResponse success_resp;
  success_resp.success = true;
  success_resp.message = "Login successful";
  success_resp.token = token;
  success_resp.id_user = id_user;
  success_resp.username = username;
  return crow::response(200, JsonHelpers::Auth::to_json(success_resp));
}

// create account

crow::response UserController::register_user(const crow::request& req) {
  std::cout << "[REGISTER] Received register request\n";

  // Parsed json BODY
  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    throw BadRequestError("Invalid JSON");
  }

  // extract the fields
  if (!json_body.has("username") || !json_body.has("email") ||
      !json_body.has("password")) {
    throw BadRequestError("Missing username, email or password");
  }

  std::string username = json_body["username"].s();
  std::string email = json_body["email"].s();
  std::string password = json_body["password"].s();

  // call service (all validation happens there)
  int64_t new_id = this->user_service.register_user(username, email, password);

  crow::json::wvalue response;
  response["success"] = true;
  response["message"] = "Account created successfully";
  response["id_user"] = new_id;

  crow::response res(201, response.dump());
  res.add_header("Content-Type", "application/json");
  return res;
}

// delete user
crow::response UserController::delete_user(int64_t id_user) {
  if (id_user == 1) {
    throw UnauthorizedError("Cannot delete this account");
  };
  std::cout << "[DELETE ACCOUNT] User " << id_user
            << " requested account deletion\n";

  std::unordered_map<int64_t, std::unordered_set<int64_t>> deleted_channels;
  std::unordered_map<int64_t, std::unordered_set<int64_t>> canceled_invitations;

  

  this->user_service.delete_user(id_user, deleted_channels,
                                 canceled_invitations);

  // TODO : create a channel deleted notification (structure and message) but
  // clients need to implement it too
  ServerSend::UserLeftNotification left_notif;
  left_notif.type = WizzMania::MessageType::USER_LEFT;
  left_notif.id_user = id_user;

  for (auto& [id_channel, users] : deleted_channels) {
    left_notif.id_channel = id_channel;
    std::string json =
        JsonHelpers::ServerSendHelpers::to_json(left_notif).dump();
    ws.broadcast_to_users(users, json);
  }

  // TODO : create a cancel invitation notification (structure and message) but
  // clients need to implement it too (if created, need to change "cancel
  // invitation structure in invitation_controller!")
  ServerSend::RejectInvitationResponse cancel_notif;
  cancel_notif.type = WizzMania::MessageType::CANCEL_INVITATION;

  cancel_notif.contact.id_user = id_user;
  cancel_notif.contact.username =
      "";  // user is gone, client only needs id_channel anyway

  for (auto& [id_channel, users] : canceled_invitations) {
    cancel_notif.id_channel = id_channel;
    std::string json =
        JsonHelpers::ServerSendHelpers::to_json(cancel_notif).dump();
    ws.broadcast_to_users(users, json);
  }

  ws.disconnect_user(id_user, "Account deleted");

  crow::json::wvalue response;
  response["success"] = true;
  response["message"] = "Account deleted successfully";

  crow::response res(200, response.dump());
  res.add_header("Content-Type", "application/json");
  return res;
}