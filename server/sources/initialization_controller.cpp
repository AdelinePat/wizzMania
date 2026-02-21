#include "initialization_controller.hpp"

void InitializationController::initial_data(crow::websocket::connection& conn, int64_t id_user) {
//   std::optional<int64_t> id_user_opt = ws_manager.get_user_id(&conn);
//   if (!id_user_opt.has_value()) {
//     std::cerr << "[INIT] Error: user not found for connection\n";
//     return;
//   }
//   int64_t id_user = id_user_opt.value();

  std::cout << "[INIT] Sending initial data to user " << id_user << "\n";

  ServerSend::InitialDataResponse init_data = this->get_initial_data(id_user);

  init_data.type = WizzMania::MessageType::INITIAL_DATA;

//   if (init_data.contacts.empty()) {
//     std::cout << "[INIT] Warning: No contacts found for user " << id_user
//               << "\n";
//   }

  std::string json_str = JsonHelpers::ServerSend::to_json(init_data).dump();
  conn.send_text(json_str);

  std::cout << "[INIT] Sent initial data: " << init_data.contacts.size()
            << " contacts, " << init_data.channels.size() << " channels, "
            << init_data.invitations.size() << " invitations\n";
}



ServerSend::InitialDataResponse InitializationController::get_initial_data(
    const int64_t id_user) {
  ServerSend::InitialDataResponse init_data;
  init_data.contacts = db.get_contacts(id_user);
  init_data.channels = db.get_initial_channels(id_user);
  init_data.invitations = db.get_initial_invitations(id_user);
  init_data.outgoing_invitations = db.get_outgoing_invitations(id_user);
  return init_data;
}
