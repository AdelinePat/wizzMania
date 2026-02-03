// common/messages.hpp
enum class ChannelStatus : uint8_t {
    PENDING = 0,
    ACCEPTED = 1,
    REJECTED = 2,
    LEFT = 3
};

// Usage in server code:
// int status_value = static_cast<int>(ChannelStatus::ACCEPTED);  // = 1
// Store status_value in DB

// Reading from DB:
// ChannelStatus status = static_cast<ChannelStatus>(row["status"].as<int>());

namespace WizzMania {

// All possible message types between client and server
enum class MessageType : uint8_t {
    // ===== Client -> Server =====
    SEND_MESSAGE = 0,           // User sends a chat message
    CREATE_CHANNEL = 1,         // User creates a new channel
    ACCEPT_INVITATION = 2,      // User accepts channel invitation
    REJECT_INVITATION = 3,      // User rejects channel invitation
    LEAVE_CHANNEL = 4,          // User leaves a channel
    UPDATE_CHANNEL_TITLE = 5,   // User changes channel title
    MARK_AS_READ = 6,           // User marks messages as read
    TYPING_START = 7,           // User starts typing
    TYPING_STOP = 8,            // User stops typing
    REQUEST_CHANNEL_HISTORY = 9,// User requests old messages
    CHANNEL_OPEN = 10           // Ask for a specific channel messages (before websocket opened) ??? A PRECISER ?
    
    // ===== Server -> Client =====
    NEW_MESSAGE = 100,          // Broadcast: New message in channel
    CHANNEL_CREATED = 101,      // Response: Channel created successfully
    CHANNEL_INVITATION = 102,   // Notification: You've been invited
    INVITATION_ACCEPTED = 103,  // Notification: Someone accepted
    INVITATION_REJECTED = 104,  // Notification: Someone rejected
    USER_JOINED = 105,          // Notification: User joined channel
    USER_LEFT = 106,            // Notification: User left channel
    CHANNEL_ACTIVATED = 107,    // Notification: Channel is now active => A PRECISER
    CHANNEL_DELETED = 108,      // Notification: Channel was deleted
    TITLE_UPDATED = 109,        // Notification: Title changed
    USER_STATUS = 110,          // Notification: User online/offline
    USER_TYPING = 111,          // Notification: Someone is typing
    INITIAL_DATA = 112,         // Response: All channels on connect
    CHANNEL_HISTORY = 113,      // Response: Old messages
    ERROR = 255                 // Error response
};

enum class InvitationStatus {
    PENDING,
    ACCEPTED,
    REJECTED,
    LEFT,
};

}
