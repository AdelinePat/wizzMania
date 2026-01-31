#ifndef HELPERS_H
#define HELPERS_H
#include "crow.h"

// Returns user ID if credentials are valid, otherwise -1
int64_t check_user_credentials(const std::string& username, const std::string& password);

// Returns a crow JSON array of channels for the user
crow::json::wvalue get_channels_for_user(int64_t user_id);

#endif