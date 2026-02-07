#ifndef SERVER_H
#define SERVER_H

// #include <chrono>
#include <crow.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "auth.hpp"
#include "helpers.hpp"
#include "json_helpers.hpp"
#include "message_handler.hpp"
#include "message_structure.hpp"
#include "websocket_manager.hpp"

using WSConn = crow::websocket::connection*;
#endif
