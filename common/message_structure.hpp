#ifndef MESSAGE_STRUCTURE_HPP
#define MESSAGE_STRUCTURE_HPP

#include <string>
#include <vector>
#include <set>
#include "messages.hpp"

// ===== LOGIN/LOGOUT ======
namespace AuthMessages {
// HTTP POST REQUEST (no type field - this is HTTP, not WebSocket)
struct LoginRequest {
  std::string username;
  std::string password;
};

struct LoginResponse {
  bool success;
  std::string message;
  std::string token;
  int64_t id_user;
  std::string username;
};

// WebSocket authentication request
struct WSAuthRequest {
  WizzMania::MessageType type;  // Client sets this to WS_AUTH
  std::string token;
};

struct WSAuthResponse {
  WizzMania::MessageType type;  // Server sets this to WS_AUTH_SUCCESS
  std::string message;
  int64_t id_user;
};

struct LogoutRequest {
  WizzMania::MessageType type;  // Client sets this to LOGOUT
  std::string reason;
};
}  // namespace AuthMessages

// ===== CLIENT -> SERVER Messages =====
namespace ClientSend {
struct SendMessageRequest {
  WizzMania::MessageType type;  // SEND_MESSAGE
  int64_t channel_id;
  std::string body;
};

struct CreateChannelRequest {
  WizzMania::MessageType type;  // CREATE_CHANNEL
  std::vector<int64_t> participant_ids;
  std::string title;
};

struct AcceptInvitationRequest {
  WizzMania::MessageType type;  // ACCEPT_INVITATION
  int64_t channel_id;
};

struct RejectInvitationRequest {
  WizzMania::MessageType type;  // REJECT_INVITATION
  int64_t channel_id;
};

struct LeaveChannelRequest {
  WizzMania::MessageType type;  // LEAVE_CHANNEL
  int64_t channel_id;
};

struct UpdateChannelTitleRequest {
  WizzMania::MessageType type;  // UPDATE_CHANNEL_TITLE
  int64_t channel_id;
  std::string new_title;
};

struct MarkAsReadRequest {
  WizzMania::MessageType type;  // MARK_AS_READ
  int64_t channel_id;
  int64_t last_id_message;
};

struct TypingRequest {
  WizzMania::MessageType type;  // TYPING_START or TYPING_STOP
  int64_t channel_id;
  bool is_typing;
};

struct RequestChannelHistoryRequest {
  WizzMania::MessageType type;  // REQUEST_CHANNEL_HISTORY
  int64_t channel_id;
  int64_t before_id_message;
  int limit = 50;  // This default makes sense - it's a parameter, not a type
};

struct ChannelOpenRequest {
  WizzMania::MessageType type;  // CHANNEL_OPEN
  int64_t channel_id;
};
}  // namespace ClientSend

// ===== SERVER -> CLIENT Messages =====
namespace ServerSend {
struct Message {
  int64_t id_message;
  int64_t id_sender;
  // std::string sender_username; // client caches id_user <-> username ?
  std::string body;
  std::string timestamp;
  bool is_system;
};

struct NewMessageBroadcast {
  WizzMania::MessageType type;  // NEW_MESSAGE
  Message message;
  int64_t channel_id;
};

struct Contact {
  int64_t id_user;
  std::string username;
  // ChannelStatus status;
  // bool is_online;
};

struct ChannelInfo {
  int64_t channel_id;
  std::string title;
  bool is_group;
  int64_t created_by;
  // std::vector<Participant> participants;
  std::set<int64_t> participants;
  Message last_message;  // display preview of last message
  int64_t unread_count;
  int64_t last_read_id_message;
};

struct ChannelCreatedResponse {
  WizzMania::MessageType type;  // CHANNEL_CREATED
  int64_t channel_id;
  bool already_existed;
  ChannelInfo channel;
};

struct ChannelInvitation {
  WizzMania::MessageType type;  // CHANNEL_INVITATION
  int64_t channel_id;
  int64_t inviter_id;
  // std::string inviter_username;
  std::vector<Contact> other_participant_ids;
  std::string title;
};

struct InvitationAcceptedNotification {
  WizzMania::MessageType type;  // INVITATION_ACCEPTED
  int64_t channel_id;
  int64_t id_user;
  std::string username;
};

struct InvitationRejectedNotification {
  WizzMania::MessageType type;  // INVITATION_REJECTED
  int64_t channel_id;
  int64_t id_user;
  std::string username;
};

struct UserJoinedNotification {
  WizzMania::MessageType type;  // USER_JOINED
  int64_t channel_id;
  int64_t id_user;
  std::string username;
};

struct UserLeftNotification {
  WizzMania::MessageType type;  // USER_LEFT
  int64_t channel_id;
  int64_t id_user;
  std::string username;
};

struct ChannelActivatedNotification {
  WizzMania::MessageType type;  // CHANNEL_ACTIVATED
  int64_t channel_id;
  std::string title;
};

struct ChannelDeletedNotification {
  WizzMania::MessageType type;  // CHANNEL_DELETED
  int64_t channel_id;
  std::string reason;
};

struct TitleUpdatedNotification {
  WizzMania::MessageType type;  // TITLE_UPDATED
  int64_t channel_id;
  std::string new_title;
  int64_t updated_by;
};

struct UserStatusNotification {
  WizzMania::MessageType type;  // USER_STATUS
  int64_t id_user;
  bool is_online;
};

struct UserTypingNotification {
  WizzMania::MessageType type;  // USER_TYPING
  int64_t channel_id;
  int64_t id_user;
  std::string username;
  bool is_typing;
};

struct InitialDataResponse {
  WizzMania::MessageType type;  // INITIAL_DATA
  std::vector<Contact> contacts;
  std::vector<ChannelInfo> channels;
  std::vector<ChannelInvitation> invitations;
};

struct ChannelHistoryResponse {
  WizzMania::MessageType type;  // CHANNEL_HISTORY
  int64_t channel_id;
  std::vector<Message> messages;
  bool has_more;
};

struct ErrorResponse {
  WizzMania::MessageType type;  // ERROR
  std::string error_code;
  std::string message;
};
}  // namespace ServerSend

#endif  // MESSAGE_STRUCTURE_HPP