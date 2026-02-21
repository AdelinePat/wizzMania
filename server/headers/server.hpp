#ifndef SERVER_H
#define SERVER_H

// #include <chrono>
#include <crow.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "channel_controller.hpp"
#include "database.hpp"
#include "helpers.hpp"
#include "invitation_controller.hpp"
#include "json_helpers.hpp"
#include "message_controller.hpp"
#include "message_handler.hpp"
#include "message_structure.hpp"
#include "user_controller.hpp"
#include "user_service.hpp"
#include "websocket_manager.hpp"

using WSConn = crow::websocket::connection*;
#endif
