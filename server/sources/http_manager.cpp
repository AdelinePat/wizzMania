#include "http_manager.hpp"

crow::response HttpManager::send_login_error(const int code,
                                             const std::string& message) {
  AuthMessages::LoginResponse error_resp;
  error_resp.success = false;
  error_resp.message = message;
  return crow::response(code, JsonHelpers::Auth::to_json(error_resp));
}

crow::response HttpManager::send_login_response(const int64_t id_user,
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

crow::response HttpManager::login(Database& db, const crow::request& req) {
  std::cout << "[LOGIN] Received login request\n";

  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    std::cout << "[LOGIN] Invalid JSON\n";
    return this->send_login_error(400, "Invalid JSON");
  }

  std::optional<AuthMessages::LoginRequest> login_req =
      JsonHelpers::Auth::parse_login_request(json_body);
  if (!login_req.has_value()) {
    std::cout << "[LOGIN] Missing required fields\n";
    return this->send_login_error(400, "Missing username or password");
  }

  std::cout << "[LOGIN] Attempting login for user: " << login_req->username
            << "\n";
  int64_t id_user = db.verify_user(login_req->username, login_req->password);

  if (id_user < 0) {
    std::cout << "[LOGIN] Invalid credentials for: " << login_req->username
              << "\n";
    return this->send_login_error(401, "Invalid username or password");
  }
  std::string token = Auth::generateToken(id_user);
  return this->send_login_response(id_user, login_req->username, token);
}