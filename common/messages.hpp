// common/messages.hpp
#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <cstdint>

enum class ChannelStatus : uint8_t {
    PENDING,
    ACCEPTED,
    REJECTED,
    LEFT
};

namespace WizzMania {

// All possible message types between client and server
enum class MessageType : uint8_t {
    // ===== Authentication =====
    WS_AUTH = 0,                // First message: WebSocket authentication with token
    LOGOUT = 1,                 // Client requests graceful disconnect
    
    // ===== Client -> Server =====
    SEND_MESSAGE = 10,          // User sends a chat message
    CREATE_CHANNEL = 11,        // User creates a new channel
    ACCEPT_INVITATION = 12,     // User accepts channel invitation
    REJECT_INVITATION = 13,     // User rejects channel invitation
    LEAVE_CHANNEL = 14,         // User leaves a channel
    UPDATE_CHANNEL_TITLE = 15,  // User changes channel title
    MARK_AS_READ = 16,          // User marks messages as read
    // TYPING_START = 17,          // User starts typing
    // TYPING_STOP = 18,           // User stops typing
    REQUEST_CHANNEL_HISTORY = 19,// User requests old messages
    CHANNEL_OPEN = 20,          // Ask for a specific channel messages
    
    // ===== Server -> Client =====
    WS_AUTH_SUCCESS = 100,      // Response: Authentication successful
    NEW_MESSAGE = 101,          // Broadcast: New message in channel
    CHANNEL_CREATED = 102,      // Response: Channel created successfully
    CHANNEL_INVITATION = 103,   // Notification: You've been invited
    INVITATION_ACCEPTED = 104,  // Notification: Someone accepted
    INVITATION_REJECTED = 105,  // Notification: Someone rejected
    USER_JOINED = 106,          // Notification: User joined channel
    USER_LEFT = 107,            // Notification: User left channel
    // CHANNEL_ACTIVATED = 108,    // Notification: Channel is now active
    // CHANNEL_DELETED = 109,      // Notification: Channel was deleted
    TITLE_UPDATED = 110,        // Notification: Title changed
    USER_STATUS = 111,          // Notification: User online/offline
    USER_TYPING = 112,          // Notification: Someone is typing
    INITIAL_DATA = 113,         // Response: All channels on connect
    CHANNEL_HISTORY = 114,      // Response: Old messages
    ERROR = 255                 // Error response
};

}

#endif // MESSAGES_HPP