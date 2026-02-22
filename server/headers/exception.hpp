#ifndef EXCEPTION_WIZZMANIA_H
#define EXCEPTION_WIZZMANIA_H

#include <exception>
#include <iostream>
#include <string>

#include "crow.h"
#include "json_helpers.hpp"
#include "message_structure.hpp"


class WizzManiaError : public std::exception {
 public:
  WizzManiaError(int code, const std::string& message)
      : code(code), message(message) {}

  int get_code() const { return code; }
  const std::string& get_message() const { return message; }
  const char* what() const noexcept override { return message.c_str(); }

  static void send_ws_error(crow::websocket::connection& conn,
                            const WizzManiaError& error) {
    ServerSend::ErrorResponse err;
    err.type = WizzMania::MessageType::ERROR;
    err.error_code = error.get_code();
    err.message = error.get_message();

    conn.send_text(JsonHelpers::ServerSend::to_json(err).dump());
  }

  static crow::response send_http_error(int code, const std::string& message) {
    crow::json::wvalue body;
    body["error"] = message;
    auto res = crow::response(code, body.dump());
    res.add_header("Content-Type", "application/json");
    return res;
  }

 private:
  int code;
  std::string message;
};

class BadRequestError : public WizzManiaError {
 public:
  BadRequestError(const std::string& msg) : WizzManiaError(400, msg) {}
};
class UnauthorizedError : public WizzManiaError {
 public:
  UnauthorizedError(const std::string& msg) : WizzManiaError(401, msg) {}
};
class ForbiddenError : public WizzManiaError {
 public:
  ForbiddenError(const std::string& msg) : WizzManiaError(403, msg) {}
};
class NotFoundError : public WizzManiaError {
 public:
  NotFoundError(const std::string& msg) : WizzManiaError(404, msg) {}
};
class InternalError : public WizzManiaError {
 public:
  InternalError(const std::string& msg) : WizzManiaError(500, msg) {}
};

#endif