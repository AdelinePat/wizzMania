#ifndef JSON_HELPERS_HPP
#define JSON_HELPERS_HPP

#include <optional>

#include "crow.h"
#include "exception.hpp"
#include "message_structure.hpp"

// ===== CROW JSON SERIALIZATION (Server-side only) =====

namespace JsonHelpers {

// ===== Auth Helpers =====
namespace Auth {
inline std::optional<::AuthMessages::LoginRequest> parse_login_request(
    const crow::json::rvalue& json) {
  if (!json.has("username") || !json.has("password")) {
    return std::nullopt;
  }
  ::AuthMessages::LoginRequest req;
  req.username = json["username"].s();
  req.password = json["password"].s();
  return req;
}

inline crow::json::wvalue to_json(const ::AuthMessages::LoginResponse& resp) {
  crow::json::wvalue json;
  json["success"] = resp.success;
  json["message"] = resp.message;
  if (resp.success) {
    json["token"] = resp.token;
    json["id_user"] = resp.id_user;
    json["username"] = resp.username;
  }
  return json;
}

inline std::optional<::AuthMessages::WSAuthRequest> parse_ws_auth_request(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("token")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::WS_AUTH)) {
    return std::nullopt;
  }

  ::AuthMessages::WSAuthRequest req;
  req.token = json["token"].s();
  return req;
}

inline crow::json::wvalue to_json(const ::AuthMessages::WSAuthResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);
  json["message"] = resp.message;
  json["id_user"] = resp.id_user;
  return json;
}

inline std::optional<::AuthMessages::LogoutRequest> parse_logout_request(
    const crow::json::rvalue& json) {
  if (!json.has("type")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::LOGOUT)) {
    return std::nullopt;
  }

  ::AuthMessages::LogoutRequest req;
  if (json.has("reason")) {
    req.reason = json["reason"].s();
  }
  return req;
}
}  // namespace Auth

// ===== ClientSend Helpers =====
namespace ClientSendHelpers {
// FROM JSON SEND MESSAGE REQUEST
inline std::optional<::ClientSend::SendMessageRequest> parse_send_message(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel") || !json.has("body")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::SEND_MESSAGE)) {
    return std::nullopt;
  }

  ::ClientSend::SendMessageRequest req;
  req.id_channel = json["id_channel"].i();
  req.body = json["body"].s();
  return req;
}

// FROM JSON MARK AS READ REQUEST
inline std::optional<::ClientSend::MarkAsRead> parse_mark_as_read(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel") ||
      !json.has("last_id_message")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::MARK_AS_READ)) {
    return std::nullopt;
  }
  ::ClientSend::MarkAsRead mark;
  mark.type = WizzMania::MessageType::MARK_AS_READ;
  mark.id_channel = json["id_channel"].i();
  mark.last_id_message = json["last_id_message"].i();
  mark.unread_count = 0;
  return mark;
}

// FROM JSON CHANNEL HISTORY REQUEST
inline std::optional<::ClientSend::ChannelHistoryRequest>
parse_request_channel_history(const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel") || !json.has("limit")) {
    return std::nullopt;
  }

  int type_int = json["type"].i();
  if (type_int !=
      static_cast<int>(WizzMania::MessageType::REQUEST_CHANNEL_HISTORY)) {
    return std::nullopt;
  }

  ::ClientSend::ChannelHistoryRequest req;
  req.id_channel = json["id_channel"].i();
  req.before_id_message =
      json.has("before_id_message") ? json["before_id_message"].i() : 0;
  req.limit = json["limit"].i();

  return req;
}

inline std::optional<::ClientSend::CreateChannelRequest> parse_create_channel(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("usernames")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::CREATE_CHANNEL)) {
    return std::nullopt;
  }

  ::ClientSend::CreateChannelRequest req;

  auto usernames = json["usernames"];
  for (size_t i = 0; i < usernames.size(); i++) {
    req.usernames.insert(usernames[i].s());
  }

  if (req.usernames.empty()) {
    return std::nullopt;
  }

  if (json.has("title")) {
    req.title = json["title"].s();
  }

  return req;
}

// FROM JSON ACCEPT INVITATION REQUEST
inline std::optional<::ClientSend::AcceptInvitationRequest>
parse_accept_invitation(const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::ACCEPT_INVITATION)) {
    return std::nullopt;
  }

  ::ClientSend::AcceptInvitationRequest req;
  req.id_channel = json["id_channel"].i();
  return req;
}

// FROM JSON REJECT INVITATION REQUEST
inline std::optional<::ClientSend::RejectInvitationRequest>
parse_reject_invitation(const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::REJECT_INVITATION)) {
    return std::nullopt;
  }

  ::ClientSend::RejectInvitationRequest req;
  req.id_channel = json["id_channel"].i();
  return req;
}

inline std::optional<::ClientSend::LeaveChannelRequest> parse_leave_channel(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("id_channel")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::LEAVE_CHANNEL)) {
    return std::nullopt;
  }

  ::ClientSend::LeaveChannelRequest req;
  req.id_channel = json["id_channel"].i();
  return req;
}

// TO JSON MARK AS READ
inline crow::json::wvalue to_json(const ::ClientSend::MarkAsRead& mark) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(mark.type);
  json["id_channel"] = mark.id_channel;
  json["last_id_message"] = mark.last_id_message;
  json["unread_count"] = mark.unread_count;
  return json;
}
}  // namespace ClientSendHelpers

// ===== ServerSend Helpers =====
namespace ServerSendHelpers {
// TO JSON MESSAGE (must be first - other to_json functions use this)
inline crow::json::wvalue to_json(const ::ServerSend::Message& msg) {
  crow::json::wvalue json;
  json["id_message"] = msg.id_message;
  json["id_sender"] = msg.id_sender;
  json["body"] = msg.body;
  json["timestamp"] = msg.timestamp;
  json["is_system"] = msg.is_system;
  return json;
}

// TO JSON CONTACT
inline crow::json::wvalue to_json(const ::ServerSend::Contact& contact) {
  crow::json::wvalue json;
  json["id_user"] = contact.id_user;
  json["username"] = contact.username;
  return json;
}

// TO JSON CHANNEL INFO
inline crow::json::wvalue to_json(const ::ServerSend::ChannelInfo& ch) {
  crow::json::wvalue json;
  json["id_channel"] = ch.id_channel;
  json["title"] = ch.title;
  json["is_group"] = ch.is_group;
  json["created_by"] = ch.created_by;
  json["unread_count"] = ch.unread_count;
  json["last_read_id_message"] = ch.last_read_id_message;
  json["last_message"] = to_json(ch.last_message);

  crow::json::wvalue::list participants_list;
  for (const ::ServerSend::Contact& participant : ch.participants) {
    participants_list.push_back(to_json(participant));
  }
  json["participants"] = std::move(participants_list);

  return json;
}

// TO JSON CHANNEL INVITATION
inline crow::json::wvalue to_json(const ::ServerSend::ChannelInvitation& inv) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(inv.type);
  json["id_channel"] = inv.id_channel;
  json["id_inviter"] = inv.id_inviter;
  json["title"] = inv.title;

  crow::json::wvalue::list participants_list;
  for (const ::ServerSend::Contact& p : inv.other_participant_ids) {
    participants_list.push_back(to_json(p));
  }
  json["other_participant_ids"] = std::move(participants_list);

  return json;
}

// TO JSON Invitation Accepted Response
inline crow::json::wvalue to_json(
    const ::ServerSend::AcceptInvitationResponse& invitation_response) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(invitation_response.type);
  json["channel"] = to_json(invitation_response.channel);
  return json;
}

// TO JSON Invitation Rejected Response
inline crow::json::wvalue to_json(
    const ::ServerSend::RejectInvitationResponse& invitation_response) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(invitation_response.type);
  json["id_channel"] = invitation_response.id_channel;
  json["contact"] = to_json(invitation_response.contact);
  return json;
}

// TO JSON User Joined Notification
inline crow::json::wvalue to_json(
    const ::ServerSend::UserJoinedNotification& joined_notification) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(joined_notification.type);
  json["id_channel"] = joined_notification.id_channel;
  json["contact"] = to_json(joined_notification.contact);
  return json;
}

// TO JSON Send Message Response
inline crow::json::wvalue to_json(
    const ::ServerSend::SendMessageResponse& broadcast) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(broadcast.type);
  json["id_channel"] = broadcast.id_channel;
  json["message"] = to_json(broadcast.message);
  return json;
}

// TO JSON Error Response
inline crow::json::wvalue to_json(const ::ServerSend::ErrorResponse& err) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(err.type);
  json["error_code"] = err.error_code;
  json["message"] = err.message;
  return json;
}

// TO JSON Create Channel Response
inline crow::json::wvalue to_json(
    const ::ServerSend::CreateChannelResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);
  json["id_channel"] = resp.id_channel;
  json["already_existed"] = resp.already_existed;
  json["channel"] = to_json(resp.channel);
  return json;
}

// TO JSON Initial Data Response
inline crow::json::wvalue to_json(
    const ::ServerSend::InitialDataResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);

  crow::json::wvalue::list contact_list;
  for (const ::ServerSend::Contact& contact : resp.contacts) {
    contact_list.push_back(to_json(contact));
  }
  json["contacts"] = std::move(contact_list);

  crow::json::wvalue::list channels_list;
  for (const ::ServerSend::ChannelInfo& channel : resp.channels) {
    channels_list.push_back(to_json(channel));
  }
  json["channels"] = std::move(channels_list);

  crow::json::wvalue::list invitations_list;
  for (const auto& inv : resp.invitations) {
    invitations_list.push_back(to_json(inv));
  }
  json["invitations"] = std::move(invitations_list);

  crow::json::wvalue::list outgoing_invitations;
  for (const ::ServerSend::ChannelInfo& outgoing_invitation :
       resp.outgoing_invitations) {
    outgoing_invitations.push_back(to_json(outgoing_invitation));
  }
  json["outgoing_invitations"] = std::move(outgoing_invitations);

  return json;
}

// TO JSON Channel History Response
inline crow::json::wvalue to_json(
    const ::ServerSend::ChannelHistoryResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);
  json["id_channel"] = resp.id_channel;
  json["has_more"] = resp.has_more;

  crow::json::wvalue::list messages_list;
  for (const auto& msg : resp.messages) {
    messages_list.push_back(to_json(msg));
  }
  json["messages"] = std::move(messages_list);

  return json;
}

// TO JSON User Left Notification
inline crow::json::wvalue to_json(
    const ::ServerSend::UserLeftNotification& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);
  json["id_channel"] = resp.id_channel;
  json["id_user"] = resp.id_user;
  return json;
}

}  // namespace ServerSendHelpers

}  // namespace JsonHelpers

#endif  // JSON_HELPERS_HPP