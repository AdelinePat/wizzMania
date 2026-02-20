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

#endif