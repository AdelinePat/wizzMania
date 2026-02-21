#ifndef HELPERS_H
#define HELPERS_H
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "crow.h"
#include "json_helpers.hpp"
#include "message_structure.hpp"

uint16_t get_server_port();
std::string get_timestamp();
ServerSend::Message create_message_struct(int64_t id_message, int64_t id_user,
                                          const std::string& body,
                                          const std::string& timestamp);
ServerSend::ChannelInvitation create_invitation_struct(
    int64_t id_channel, int64_t id_inviter,
    const std::vector<ServerSend::Contact>& other_participants,
    std::string& title);
ServerSend::ChannelInfo create_empty_channel_info_struct(
    int64_t id_channel, int64_t created_by,
    const std::vector<ServerSend::Contact>& other_participants,
    std::string& title);

void send_error(crow::websocket::connection& conn,
                const std::string& error_code,
                const std::string& error_message);

#endif