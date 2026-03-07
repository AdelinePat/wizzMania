#include "server.hpp"

// --------------------
// CORS Middleware
// --------------------
struct CORS {
  struct context {};

  void before_handle(crow::request&, crow::response& res, context&) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "*");
    res.add_header("Access-Control-Allow-Headers", "*");
    res.add_header("Access-Control-Expose-Headers", "*");
    res.add_header("Access-Control-Max-Age", "86400");
  }

  void after_handle(crow::request&, crow::response& res, context&) {
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "*");
    res.add_header("Access-Control-Allow-Headers", "*");
    res.add_header("Access-Control-Expose-Headers", "*");
  }
};

// --------------------
// Main
// --------------------
int main() {
  std::cout << "========================================\n";
  std::cout << " WizzMania Server Starting...\n";
  std::cout << "========================================\n";

  uint16_t port = Utils::get_server_port();
  Database db;
  std::cout << "[Server] Database initialized successfully" << std::endl;

  crow::App<CORS> app;

  WebSocketManager ws_manager;

  UserController user_controller(db, ws_manager);
  MessageController message_controller(db, ws_manager);
  InvitationController invitation_controller(db, ws_manager);
  ChannelController channel_controller(db, ws_manager);
  AuthController auth_controller(ws_manager);
  InitializationController init_controller(db, ws_manager);

  // ===== OPTIONS for CORS preflight =====
  CROW_ROUTE(app, "/<path>")
      .methods("OPTIONS"_method)([](const crow::request& /*req*/,
                                    crow::response& res, std::string /*path*/) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "*");
        res.add_header("Access-Control-Allow-Headers", "*");
        res.add_header("Access-Control-Expose-Headers", "*");
        res.code = 204;
        res.end();
      });

  // ===== POST /login endpoint =====
  CROW_ROUTE(app, "/login")
      .methods("POST"_method)([&user_controller](const crow::request& req) {
        try {
          return user_controller.login(req);
        } catch (WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[LOGIN] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== POST /register endpoint =====
  CROW_ROUTE(app, "/register")
      .methods("POST"_method)([&user_controller](const crow::request& req) {
        try {
          return user_controller.register_user(req);
        } catch (WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[REGISTER] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  //====DELETE / account endpoint ====
  CROW_ROUTE(app, "/account")
      .methods("DELETE"_method)(
          [&user_controller, &auth_controller](const crow::request& req) {
            try {
              int64_t id_user = auth_controller.authenticate_http(req);
              return user_controller.delete_user(id_user);
            } catch (const WizzManiaError& e) {
              return crow::response(e.get_code(), e.get_message());
            } catch (const std::exception& e) {
              std::cerr << "[DELETE ACCOUNT] Unexpected error: " << e.what()
                        << "\n";
              return crow::response(500, e.what());
            }
          });

  // ===== PATCH / /invitation/id_channel/accept endpoint =====
  CROW_ROUTE(app, "/invitations/<int>/accept")
      .methods("PATCH"_method)([&invitation_controller, &auth_controller](
                                   const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return invitation_controller.accept_invitation(
              id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[INVITATION] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== PATCH / /invitation/id_channel/reject endpoint =====
  CROW_ROUTE(app, "/invitations/<int>/reject")
      .methods("PATCH"_method)([&invitation_controller, &auth_controller](
                                   const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return invitation_controller.reject_invitation(
              id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[REJECT INVITATION] Unexpected error: " << e.what()
                    << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== PATCH / /invitations/id_channel/cancel endpoint =====
  CROW_ROUTE(app, "/invitations/<int>/cancel")
      .methods("PATCH"_method)([&invitation_controller, &auth_controller](
                                   const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return invitation_controller.cancel_invitation(
              id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[CANCEL INVITATION] Unexpected error: " << e.what()
                    << "\n";
          return crow::response(500, e.what());
        }
      });

  //  ===== PATCH Create a channel (send new invitation)  =====
  CROW_ROUTE(app, "/channels")
      .methods("POST"_method)(
          [&channel_controller, &auth_controller](const crow::request& req) {
            try {
              int64_t id_user = auth_controller.authenticate_http(req);
              std::string token = req.get_header_value("X-Auth-Token");
              return channel_controller.create_channel(id_user, req, token);
            } catch (const WizzManiaError& e) {
              return crow::response(e.get_code(), e.get_message());
            } catch (const std::exception& e) {
              std::cerr << "[CREATE CHANNEL] Unexpected error: " << e.what()
                        << "\n";
              return crow::response(500, e.what());
            }
          });

  // ===== PATCH / /channels/id_channel/leave endpoint =====
  CROW_ROUTE(app, "/channels/<int>/leave")
      .methods("PATCH"_method)([&channel_controller, &auth_controller](
                                   const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return channel_controller.leave_channel(
              id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[LEAVE CHANNEL] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== POST /channels/<id>/messages =====
  CROW_ROUTE(app, "/channels/<int>/messages")
      .methods("POST"_method)([&message_controller, &auth_controller](
                                  const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return message_controller.send_message(
              req, id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[SEND MESSAGE] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== GET / /channels/id_channel/history endpoint =====
  CROW_ROUTE(app, "/channels/<int>/history")
      .methods("GET"_method)([&message_controller, &auth_controller](
                                 const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          return message_controller.get_history(
              req, id_user, static_cast<int64_t>(id_channel));
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[GET HISTORY] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== PATCH /channels/<id>/read =====
  CROW_ROUTE(app, "/channels/<int>/read")
      .methods("PATCH"_method)([&message_controller, &auth_controller](
                                   const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return message_controller.mark_as_read(
              req, id_user, static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[MARK AS READ] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== POST /channels/<id>/wizz =====
  CROW_ROUTE(app, "/channels/<int>/wizz")
      .methods("POST"_method)([&message_controller, &auth_controller](
                                  const crow::request& req, int id_channel) {
        try {
          int64_t id_user = auth_controller.authenticate_http(req);
          std::string token = req.get_header_value("X-Auth-Token");
          return message_controller.wizz(id_user,
                                         static_cast<int64_t>(id_channel), token);
        } catch (const WizzManiaError& e) {
          return crow::response(e.get_code(), e.get_message());
        } catch (const std::exception& e) {
          std::cerr << "[WIZZ] Unexpected error: " << e.what() << "\n";
          return crow::response(500, e.what());
        }
      });

  // ===== POST / /logout endpoint =====
  CROW_ROUTE(app, "/logout")
      .methods("POST"_method)(
          [&user_controller, &auth_controller](const crow::request& req) {
            try {
              // int64_t id_user = auth_controller.authenticate_http(req);
              int64_t id_user = auth_controller.authenticate_http(req);
              std::string token = req.get_header_value("X-Auth-Token");
              // auth_controller.authenticate_http(req);
              return user_controller.logout(token);
            } catch (const WizzManiaError& e) {
              return crow::response(e.get_code(), e.get_message());
            } catch (const std::exception& e) {
              std::cerr << "[LOGOUT] Unexpected error: " << e.what() << "\n";
              return crow::response(500, e.what());
            }
          });

  // ===== WebSocket endpoint =====
  CROW_ROUTE(app, "/ws")
      .websocket(&app)
      .onopen([&]([[maybe_unused]] crow::websocket::connection& conn) {
        std::cout
            << "[WS] New WebSocket connection opened (waiting for auth)\n";
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason,
                   uint16_t /*status_code*/) {
        std::optional<int64_t> id_user = ws_manager.get_id_user(&conn);
        if (id_user.has_value()) {
          std::cout << "[WS] User " << id_user.value()
                    << " disconnected. Reason: " << reason << "\n";
        } else {
          std::cout << "[WS] Unauthenticated connection closed. Reason: "
                    << reason << "\n";
        }
        ws_manager.remove_connection(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data,
                     [[maybe_unused]] bool is_binary) {
        crow::json::rvalue json_msg = crow::json::load(data);
        if (!json_msg || !json_msg.has("type")) {
          std::cout << "[WS] Invalid JSON or missing 'type' field\n";
          conn.close("Invalid message format");
          return;
        }

        int type_int = json_msg["type"].i();
        WizzMania::MessageType msg_type =
            static_cast<WizzMania::MessageType>(type_int);

        // ===== AUTHENTICATION =====
        if (msg_type == WizzMania::MessageType::WS_AUTH) {
          try {
            int64_t id_user = auth_controller.authenticate_ws(conn, json_msg);
            return init_controller.initial_data(conn, id_user);
          } catch (const WizzManiaError& e) {
            std::cerr << e.get_message();
            auth_controller.auth_error(conn, "Invalid authentication format");
          }
        }

        // ===== ALL OTHER MESSAGES REQUIRE AUTH =====
        std::optional<int64_t> id_user_opt = ws_manager.get_id_user(&conn);
        if (!id_user_opt.has_value()) {
          std::cout << "[WS] Unauthenticated message attempt\n";
          conn.close("Authentication required");
          return;
        }

        // ===== ROUTE AUTHENTICATED MESSAGES =====
        int64_t id_user = id_user_opt.value();
        std::cout << "[WS] User " << id_user << " - type: " << type_int << "\n";

        switch (msg_type) {
            // case WizzMania::MessageType::SEND_MESSAGE: {
            //   message_controller.send_message(conn, id_user, json_msg);
            //   break;
            // }

            // case WizzMania::MessageType::MARK_AS_READ: {
            //   message_controller.mark_as_read(conn, id_user, json_msg);
            //   break;
            // }

          // case WizzMania::MessageType::WIZZ: {
          //   message_controller.wizz(conn, id_user, json_msg);
          //   break;
          // }

            // case WizzMania::MessageType::TYPING_START:
            // case WizzMania::MessageType::TYPING_STOP: {
            //   auto req =
            //   JsonHelpers::ClientSendHelpers::parse_typing(json_msg); if
            //   (!req.has_value()) {
            //     ServerSend::ErrorResponse err;
            //     err.type = WizzMania::MessageType::ERROR;
            //     err.message = "Invalid TYPING format";
            //     err.error_code = "INVALID_FORMAT";
            //     conn.send_text(JsonHelpers::ServerSendHelpers::to_json(err).dump());
            //     return;
            //   }

            //   std::cout << "[TYPING] User " << id_user << " in channel "
            //             << req->id_channel << ": "
            //             << (req->is_typing ? "start" : "stop") << "\n";

            //   // TODO: Broadcast typing

            //   break;
            // }

            // case WizzMania::MessageType::LOGOUT: {
            //   auto req = JsonHelpers::Auth::parse_logout_request(json_msg);
            //   std::cout << "[LOGOUT] User " << id_user;
            //   if (req.has_value() && !req->reason.empty()) {
            //     std::cout << " (" << req->reason << ")";
            //   }
            //   std::cout << "\n";

            //   conn.close("User logout");
            //   break;
            // }

          default:
            std::cout << "[WS] Unhandled type: " << type_int << "\n";
            ServerSend::ErrorResponse err;
            err.type = WizzMania::MessageType::ERROR;
            err.message = "Message type not implemented SERVER SIDE!";
            err.error_code = "NOT_IMPLEMENTED";
            conn.send_text(JsonHelpers::ServerSendHelpers::to_json(err).dump());
            break;
        }
      });

  std::cout << "[INFO] Server listening on port " << port << "\n";
  std::cout << "========================================\n";

  app.port(port).multithreaded().run();

  return 0;
}