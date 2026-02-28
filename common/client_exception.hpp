#ifndef CLIENT_EXCEPTION_WIZZMANIA_H
#define CLIENT_EXCEPTION_WIZZMANIA_H

#include <exception>
#include <iostream>
#include <string>

class WizzManiaClientError : public std::exception {
 public:
  WizzManiaClientError(const std::string& message)
      : message(message) {}

  const std::string& get_message() const { return message; }
  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};


class BadInput : public WizzManiaClientError {
 public:
  BadInput(const std::string& msg) : WizzManiaClientError(msg) {}
};

#endif