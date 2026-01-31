#include "helpers.hpp"

// --------------------
// Helper stubs (replace with DB logic later)
// --------------------
int64_t check_user_credentials(const std::string& username, const std::string& password) {
    if (username == "alice" && password == "hash") return 1;
    if (username == "bob"   && password == "hash") return 2;
    if (username == "carol" && password == "hash") return 3;
    return -1;
}

crow::json::wvalue get_channels_for_user(int64_t user_id) {
    crow::json::wvalue channels;

    if (user_id == 1) {
        channels[0]["id_channel"] = 1;
        channels[0]["title"] = "Chat with Bob";
        channels[1]["id_channel"] = 2;
        channels[1]["title"] = "Chat with Carol";
    } else if (user_id == 2) {
        channels[0]["id_channel"] = 1;
        channels[0]["title"] = "Chat with Alice";
    } else if (user_id == 3) {
        channels[0]["id_channel"] = 2;
        channels[0]["title"] = "Chat with Alice";
    }

    return channels;
}