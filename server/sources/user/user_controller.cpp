
#include "user_controller.hpp"

crow::response UserController::login(const crow::request& req) {
  std::cout << "[LOGIN] Received login request\n";

  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    std::cout << "[LOGIN] Invalid JSON\n";
    BadRequestError error = BadRequestError("Invalid JSON");
    return this->send_login_error(error);
  }

  std::optional<AuthMessages::LoginRequest> login_req =
      JsonHelpers::Auth::parse_login_request(json_body);
  if (!login_req.has_value()) {
    std::cout << "[LOGIN] Missing required fields\n";
    BadRequestError error = BadRequestError("Missing username or password");
    return this->send_login_error(error);
  }

  try {
    int64_t id_user = this->user_service.login(login_req.value());
    std::string token = auth_controller.generateToken(id_user);
    return this->send_login_response(id_user, login_req->username, token);

  } catch (const WizzManiaError& e) {
    return this->send_login_error(e);
  }
}

crow::response UserController::logout(const crow::request& req) {
  std::cout << "[LOGOUT] Received logout request\n";
  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    std::cout << "[LOGIN] Invalid JSON\n";
    BadRequestError error = BadRequestError("Invalid JSON");
    return this->send_login_error(error);
  }

  // try {
  std::string token = req.get_header_value("X-Auth-Token");
  ws_manager.disconnect_token(token);
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
