#include "initialization_controller.hpp"

void InitializationController::initial_data(crow::websocket::connection& conn,
                                            int64_t id_user) {
  try {
    std::cout << "[INIT] Sending initial data to user " << id_user << "\n";

    ServerSend::InitialDataResponse init_data = this->get_initial_data(id_user);

    init_data.type = WizzMania::MessageType::INITIAL_DATA;

    std::string json_str =
        JsonHelpers::ServerSendHelpers::to_json(init_data).dump();
    conn.send_text(json_str);

    std::cout << "[INIT] Sent initial data: " << init_data.contacts.size()
              << " contacts, " << init_data.channels.size() << " channels, "
              << init_data.invitations.size() << " invitations\n";
  } catch (const WizzManiaError& e) {
    crow::json::wvalue err;
    err["type"] = static_cast<int>(WizzMania::MessageType::ERROR);
    err["message"] = e.get_message();
    conn.send_text(err.dump());
  }
}

ServerSend::InitialDataResponse InitializationController::get_initial_data(
    const int64_t id_user) {
  ServerSend::InitialDataResponse init_data;
  //   init_data.contacts = db.get_user_contacts(id_user);
  init_data.contacts = user_service.get_all_user_contacts(id_user);
  init_data.channels = channel_service.get_all_user_channels(id_user);
  init_data.invitations =
      invitation_service.get_all_user_incoming_invitations(id_user);
  init_data.outgoing_invitations =
      invitation_service.get_all_outgoing_invitations(id_user);
  return init_data;
}
