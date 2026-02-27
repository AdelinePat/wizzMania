
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
  std::string token = auth_controller.generateToken(id_user);
  return this->send_login_response(id_user, login_req->username, token);
}

crow::response UserController::logout(int64_t id_user,
                                      const std::string& token) {
  std::cout << "[LOGOUT] Received logout request\n";
  // crow::json::rvalue json_body = crow::json::load(req.body);
  // if (!json_body) {
  //   std::cout << "[LOGIN] Invalid JSON\n";
  //   BadRequestError error = BadRequestError("Invalid JSON");
  //   return this->send_login_error(error);
  // }

  // try {
  // std::string token = req.get_header_value("X-Auth-Token");
  // if (token.empty()) {
  //   throw UnauthorizedError("Missing X-Auth-Token header");
  // }
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
    // return WizzManiaError::send_http_error(400, "Invalid JSON");
    throw BadRequestError("Invalid JSON");
  }

  // extract the fields
  if (!json_body.has("username") || !json_body.has("email") ||
      !json_body.has("password")) {
    // return WizzManiaError::send_http_error(
    //     400, "Missing username, email or password");
    throw BadRequestError("Missing username, email or password");
  }

  std::string username = json_body["username"].s();
  std::string email = json_body["email"].s();
  std::string password = json_body["password"].s();

  // call service (all validation happens there)
  //  try
  //  {
  int64_t new_id = this->user_service.register_user(username, email, password);

  crow::json::wvalue response;
  response["success"] = true;
  response["message"] = "Account created successfully";
  response["id_user"] = new_id;

  crow::response res(201, response.dump());
  res.add_header("Content-Type", "application/json");
  return res;

  // }
  //  catch (const WizzManiaError& e) {
  //     return WizzManiaError::send_http_error(e.get_code(), e.get_message());
  //   }
}

// delete user
crow::response UserController::delete_user(int64_t id_user) {
  std::cout << "[DELETE ACCOUNT] User " << id_user
            << " requested account deletion\n";
  // try
  // {
  this->user_service.delete_user(id_user);

  // Disconnect all webSocket sessions for this user
  // std::vector<WSConn> connections = ws.get_user_connections(id_user);
  // DANGEROUS, no guard lock, raw manipulation , need to use ws_manager instead
  // for(WSConn conn : connections) {
  //   conn->close("Account deleted");
  // }
  ws.disconnect_user(id_user, "Account deleted");

  crow::json::wvalue response;
  response["success"] = true;
  response["message"] = "Account deleted successfully";

  crow::response res(200, response.dump());
  res.add_header("Content-Type", "application/json");
  return res;

  // }
  // catch(const WizzManiaError& e)
  // {
  //   return WizzManiaError::send_http_error(e.get_code(), e.get_message());
  // }
}