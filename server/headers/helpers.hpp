#ifndef HELPERS_H
#define HELPERS_H

#include <string>

#include "crow.h"
#include "json_helpers.hpp"
#include "message_structure.hpp"
#include "exception.hpp"

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

crow::response send_http_error(int code, const std::string& message);

#endif