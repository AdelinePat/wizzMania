#include "auth_controller.hpp"

std::string AuthController::generateToken(int64_t id_user) {
  auto now = std::chrono::system_clock::now();
  auto expiration = now + std::chrono::hours(24 * 7);  // 7 days
  return auth_service.create_token(now, expiration, id_user);
}

void AuthController::authenticate_ws(crow::websocket::connection& conn,
                                     const crow::json::rvalue& json_msg) {
  if (ws_manager.is_authenticated(&conn)) {
    std::cout << "[WS] User already authenticated\n";
    return;
  }

  std::cout << "[WS] Processing authentication request\n";

  std::optional<::AuthMessages::WSAuthRequest> auth_req =
      JsonHelpers::Auth::parse_ws_auth_request(json_msg);

  if (!auth_req.has_value()) {
    this->auth_error(conn, "Invalid authentication format");
    return;
  }

  int64_t validated_id_user = auth_service.validate_token(auth_req->token);

  this->auth_success(conn, validated_id_user);

  // TODO
  // this->initial_data(conn);

  return;
}

void AuthController::auth_success(crow::websocket::connection& conn,
                                  const int64_t validated_id_user) {
  std::cout << "[WS] ✅ User " << validated_id_user << " authenticated!\n";

  ws_manager.add_user(validated_id_user, &conn);

  AuthMessages::WSAuthResponse auth_resp;
  auth_resp.type =
      WizzMania::MessageType::WS_AUTH_SUCCESS;  // Explicitly set type
  auth_resp.message = "Authentication successful";
  auth_resp.id_user = validated_id_user;
  conn.send_text(JsonHelpers::Auth::to_json(auth_resp).dump());
}

void AuthController::auth_error(crow::websocket::connection& conn,
                                const std::string& message) {
  std::cout << "[WS]" << message << "\n";
  conn.close(message);
}