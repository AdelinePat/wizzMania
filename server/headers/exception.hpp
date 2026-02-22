#ifndef EXCEPTION_WIZZMANIA_H
#define EXCEPTION_WIZZMANIA_H

#include <exception>
#include <iostream>
#include <string>

// class HttpError : public std::exception {
//  private:
//   int error_code;
//   std::string message;
//   std::string full_message;

//  public:
//   HttpError(int code, const std::string& msg)
//       : error_code(code),
//         message(msg),
//         full_message(std::to_string(code) + " : " + msg) {}

//   const char* what() const noexcept override { return full_message.c_str(); }
//   int get_code() const { return error_code; }
//   std::string get_message() const { return message; }
//   std::string get_full_message() const { return full_message; }
// };

// class WsError : public std::exception {
//  private:
//   std::string message;

//  public:
//   WsError(const std::string& msg) : message(msg) {}

//   const char* what() const noexcept override { return message.c_str(); }
//   std::string get_message() const { return message; }
// };




class WizzManiaError : public std::exception {
public:
    WizzManiaError(int code, const std::string& message)
        : code(code), message(message) {}

    int get_code() const { return code; }
    const std::string& get_message() const { return message; }
    const char* what() const noexcept override { return message.c_str(); }

private:
    int code;
    std::string message;
};

class BadRequestError   : public WizzManiaError { public: BadRequestError(const std::string& msg)   : WizzManiaError(400, msg) {} };
class UnauthorizedError : public WizzManiaError { public: UnauthorizedError(const std::string& msg) : WizzManiaError(401, msg) {} };
class ForbiddenError    : public WizzManiaError { public: ForbiddenError(const std::string& msg)    : WizzManiaError(403, msg) {} };
class NotFoundError     : public WizzManiaError { public: NotFoundError(const std::string& msg)     : WizzManiaError(404, msg) {} };
class InternalError     : public WizzManiaError { public: InternalError(const std::string& msg)     : WizzManiaError(500, msg) {} };

#endif