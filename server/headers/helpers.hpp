#ifndef HELPERS_H
#define HELPERS_H
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
// #include "json_helpers.hpp"
#include "crow.h"
#include "message_structure.hpp"

uint16_t get_server_port();
std::string get_timestamp();
ServerSend::Message create_message(int64_t id_message, int64_t id_user,
                                   const std::string& body, const std::string& timestamp);

#endif