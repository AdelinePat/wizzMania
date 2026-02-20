#include "user_service.hpp"


int64_t UserService::login(AuthMessages::LoginRequest login_request) {
  std::cout << "[LOGIN] Attempting login for user: " << login_request.username
            << "\n";
  int64_t id_user = db.verify_user(login_request.username, login_request.password);

  if (id_user < 0) {
    std::cout << "[LOGIN] Invalid credentials for: " << login_request.username
              << "\n";
    // return this->send_login_error(401, "Invalid username or password");
    throw HttpError(401, "Invalid username or password");
  }
  return id_user;
}