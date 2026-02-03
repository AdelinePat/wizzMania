
// ===== CLIENT -> SERVER Messages =====
namespace CliendSend {
    struct SendMessageRequest {
        int64_t channel_id;
        std::string body;
    };

    struct CreateChannelRequest {
        std::vector<int64_t> participant_ids;  // User IDs to invite
        std::string title;                      // Optional, can be empty
    };

    struct AcceptInvitationRequest {
        int64_t channel_id;
    };

    struct RejectInvitationRequest {
        int64_t channel_id;
    };

    struct LeaveChannelRequest {
        int64_t channel_id;
    };

    struct UpdateChannelTitleRequest {
        int64_t channel_id;
        std::string new_title;
    };

    struct MarkAsReadRequest {
        int64_t channel_id;
        int64_t last_message_id;  // Mark all messages up to this ID as read
    };

    struct TypingRequest {
        int64_t channel_id;
        bool is_typing;  // true = started, false = stopped
    };

    struct RequestChannelHistoryRequest {
        int64_t channel_id;
        int64_t before_message_id;  // Get messages before this ID (pagination)
        int limit = 50;             // How many messages to fetch
    };
}


// ===== SERVER -> CLIENT Messages =====
namespace ServerSend {
    struct Message {
        int64_t message_id;
        int64_t sender_id;
        std::string sender_username;
        std::string body;
        std::string timestamp;  // ISO 8601: "2026-02-01T14:30:00Z"
        bool is_system;         // true if sender_id = 0
    };

    struct NewMessageBroadcast {
        Message message;
        int64_t channel_id;
    };

    struct Participant {
        int64_t user_id;
        std::string username;
        ChannelStatus status;
        bool is_online;
    };

    struct ChannelInfo {
        int64_t channel_id;
        std::string title;
        bool is_group;
        int64_t created_by;
        std::vector<Participant> participants;
        Message last_message;       // Most recent message
        int64_t unread_count;       // Number of unread messages
        int64_t last_read_message_id;
    };

    struct ChannelCreatedResponse {
        int64_t channel_id;
        bool already_existed;       // true if channel already existed
        ChannelInfo channel;
    };

    struct ChannelInvitation {
        int64_t channel_id;
        int64_t inviter_id;
        std::string inviter_username;
        std::vector<int64_t> other_participant_ids;
        std::string title;
    };

    struct InvitationAcceptedNotification {
        int64_t channel_id;
        int64_t user_id;
        std::string username;
    };

    struct InvitationRejectedNotification {
        int64_t channel_id;
        int64_t user_id;
        std::string username;
    };

    struct UserJoinedNotification {
        int64_t channel_id;
        int64_t user_id;
        std::string username;
    };

    struct UserLeftNotification {
        int64_t channel_id;
        int64_t user_id;
        std::string username;
    };

    struct ChannelActivatedNotification {
        int64_t channel_id;
        std::string title;
    };

    struct ChannelDeletedNotification {
        int64_t channel_id;
        std::string reason;
    };

    struct TitleUpdatedNotification {
        int64_t channel_id;
        std::string new_title;
        int64_t updated_by;
    };

    struct UserStatusNotification {
        int64_t user_id;
        bool is_online;
    };

    struct UserTypingNotification {
        int64_t channel_id;
        int64_t user_id;
        std::string username;
        bool is_typing;
    };

    struct InitialDataResponse {
        std::vector<ChannelInfo> channels;           // Accepted channels
        std::vector<ChannelInvitation> invitations;  // Pending invitations
    };

    struct ChannelHistoryResponse {
        int64_t channel_id;
        std::vector<Message> messages;
        bool has_more;  // true if there are older messages
    };

    struct ErrorResponse {
        std::string error_code;     // e.g., "UNAUTHORIZED", "CHANNEL_NOT_FOUND"
        std::string message;        // Human-readable error
    };

}
