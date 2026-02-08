#include "http_manager.hpp"

crow::response HttpManager::login(Database& db, const crow::request& req) {
  std::cout << "[LOGIN] Received login request\n";

  crow::json::rvalue json_body = crow::json::load(req.body);
  if (!json_body) {
    std::cout << "[LOGIN] Invalid JSON\n";
    AuthMessages::LoginResponse error_resp;
    error_resp.success = false;
    error_resp.message = "Invalid JSON";
    return crow::response(400, JsonHelpers::Auth::to_json(error_resp));
  }

  std::optional<AuthMessages::LoginRequest> login_req =
      JsonHelpers::Auth::parse_login_request(json_body);
  if (!login_req.has_value()) {
    std::cout << "[LOGIN] Missing required fields\n";
    AuthMessages::LoginResponse error_resp;
    error_resp.success = false;
    error_resp.message = "Missing username or password";
    return crow::response(400, JsonHelpers::Auth::to_json(error_resp));
  }

  std::cout << "[LOGIN] Attempting login for user: " << login_req->username
            << "\n";

  int64_t user_id = db.verify_user(login_req->username, login_req->password);

  if (user_id < 0) {
    std::cout << "[LOGIN] Invalid credentials for: " << login_req->username
              << "\n";
    AuthMessages::LoginResponse error_resp;
    error_resp.success = false;
    error_resp.message = "Invalid username or password";
    return crow::response(401, JsonHelpers::Auth::to_json(error_resp));
  }

  std::string token = Auth::generateToken(user_id);
  std::cout << "[LOGIN] Login successful! User ID: " << user_id << "\n";
  std::cout << "[LOGIN] Token generated: " << token.substr(0, 20) << "...\n";

  AuthMessages::LoginResponse success_resp;
  success_resp.success = true;
  success_resp.message = "Login successful";
  success_resp.token = token;
  success_resp.user_id = user_id;
  success_resp.username = login_req->username;

  return crow::response(200, JsonHelpers::Auth::to_json(success_resp));
}