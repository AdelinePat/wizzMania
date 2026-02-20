#ifndef EXCEPTION_WIZZMANIA_H
#define EXCEPTION_WIZZMANIA_H

#include <exception>
#include <iostream>
#include <string>

class HttpError : public std::exception {
 private:
  int error_code;
  std::string message;
  std::string full_message;

 public:
  HttpError(int code, const std::string& msg)
      : error_code(code),
        message(msg),
        full_message(std::to_string(code) + " : " + msg) {}

  const char* what() const noexcept override { return full_message.c_str(); }
  int get_code() const { return error_code; }
  std::string get_message() const { return message; }
  std::string get_full_message() const { return full_message; }
};

#endif