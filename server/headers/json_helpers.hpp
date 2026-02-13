#ifndef JSON_HELPERS_HPP
#define JSON_HELPERS_HPP

#include <optional>

#include "crow.h"
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
  json["type"] = static_cast<int>(resp.type);  // Cast enum to int for JSON
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
namespace ClientSend {
inline std::optional<::ClientSend::SendMessageRequest> parse_send_message(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("channel_id") || !json.has("body")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::SEND_MESSAGE)) {
    return std::nullopt;
  }

  ::ClientSend::SendMessageRequest req;
  req.channel_id = json["channel_id"].i();
  req.body = json["body"].s();
  return req;
}

inline std::optional<::ClientSend::CreateChannelRequest> parse_create_channel(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("participant_ids")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::CREATE_CHANNEL)) {
    return std::nullopt;
  }

  ::ClientSend::CreateChannelRequest req;

  auto participants = json["participant_ids"];
  for (size_t i = 0; i < participants.size(); i++) {
    req.participant_ids.push_back(participants[i].i());
  }

  if (json.has("title")) {
    req.title = json["title"].s();
  }
  return req;
}

inline std::optional<::ClientSend::TypingRequest> parse_typing(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("channel_id") || !json.has("is_typing")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  auto msg_type = static_cast<WizzMania::MessageType>(type_int);
  if (msg_type != WizzMania::MessageType::TYPING_START &&
      msg_type != WizzMania::MessageType::TYPING_STOP) {
    return std::nullopt;
  }

  ::ClientSend::TypingRequest req;
  req.type = static_cast<WizzMania::MessageType>(type_int);
  req.channel_id = json["channel_id"].i();
  req.is_typing = json["is_typing"].b();
  return req;
}

inline std::optional<::ClientSend::AcceptInvitationRequest>
parse_accept_invitation(const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("channel_id")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::ACCEPT_INVITATION)) {
    return std::nullopt;
  }

  ::ClientSend::AcceptInvitationRequest req;
  req.channel_id = json["channel_id"].i();
  return req;
}

inline std::optional<::ClientSend::LeaveChannelRequest> parse_leave_channel(
    const crow::json::rvalue& json) {
  if (!json.has("type") || !json.has("channel_id")) {
    return std::nullopt;
  }
  int type_int = json["type"].i();
  if (type_int != static_cast<int>(WizzMania::MessageType::LEAVE_CHANNEL)) {
    return std::nullopt;
  }

  ::ClientSend::LeaveChannelRequest req;
  req.channel_id = json["channel_id"].i();
  return req;
}
}  // namespace ClientSend

// ===== ServerSend Helpers =====
namespace ServerSend {
inline crow::json::wvalue to_json(const ::ServerSend::Message& msg) {
  crow::json::wvalue json;
  json["message_id"] = msg.message_id;
  json["sender_id"] = msg.sender_id;
  // json["sender_username"] = msg.sender_username;
  json["body"] = msg.body;
  json["timestamp"] = msg.timestamp;
  json["is_system"] = msg.is_system;
  return json;
}

inline crow::json::wvalue to_json(const ::ServerSend::ChannelInfo& ch) {
  crow::json::wvalue json;
  json["channel_id"] = ch.channel_id;
  json["title"] = ch.title;
  json["is_group"] = ch.is_group;
  json["created_by"] = ch.created_by;
  json["unread_count"] = ch.unread_count;
  json["last_read_message_id"] = ch.last_read_message_id;
  json["last_message"] = to_json(ch.last_message);

  // crow::json::wvalue::list participants_list;
  // for (const auto& p : ch.participants) {
  //   participants_list.push_back(to_json(p));
  // }
  // json["participants"] = std::move(participants_list);

  return json;
}

// TO JSON CONTACT
inline crow::json::wvalue to_json(const ::ServerSend::Contact& contact) {
  crow::json::wvalue json;
  json["id_user"] = contact.id_user;
  json["username"] = contact.username;

  //  id_user;
  // std::string username;

  // crow::json::wvalue::list participants_list;
  // for (const auto& p : ch.participants) {
  //   participants_list.push_back(to_json(p));
  // }
  // json["participants"] = std::move(participants_list);

  return json;
}

inline crow::json::wvalue to_json(
    const ::ServerSend::NewMessageBroadcast& broadcast) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(broadcast.type);  // Cast enum to int
  json["channel_id"] = broadcast.channel_id;
  json["message"] = to_json(broadcast.message);
  return json;
}

inline crow::json::wvalue to_json(
    const ::ServerSend::UserTypingNotification& notif) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(notif.type);  // Cast enum to int
  json["channel_id"] = notif.channel_id;
  json["id_user"] = notif.id_user;
  json["username"] = notif.username;
  json["is_typing"] = notif.is_typing;
  return json;
}

inline crow::json::wvalue to_json(const ::ServerSend::ErrorResponse& err) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(err.type);  // Cast enum to int
  json["error_code"] = err.error_code;
  json["message"] = err.message;
  return json;
}

inline crow::json::wvalue to_json(
    const ::ServerSend::ChannelCreatedResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);  // Cast enum to int
  json["channel_id"] = resp.channel_id;
  json["already_existed"] = resp.already_existed;
  json["channel"] = to_json(resp.channel);
  return json;
}

inline crow::json::wvalue to_json(
    const ::ServerSend::InitialDataResponse& resp) {
  crow::json::wvalue json;
  json["type"] = static_cast<int>(resp.type);  // Cast enum to int

  // contacts
  crow::json::wvalue::list contact_list;
  for (const ::ServerSend::Contact& contact : resp.contacts) {
    contact_list.push_back(to_json(contact));
  }
  json["contacts"] = std::move(contact_list);

  // Channels
  crow::json::wvalue::list channels_list;
  for (const ::ServerSend::ChannelInfo& channel : resp.channels) {
    channels_list.push_back(to_json(channel));
  }
  json["channels"] = std::move(channels_list);

  // Invitations (TODO later)
  // crow::json::wvalue::list invitations_list;
  // for (const auto& inv : resp.invitations) {
  //   invitations_list.push_back(to_json(inv));
  // }
  // json["invitations"] = std::move(invitations_list);

  return json;
}
}  // namespace ServerSend

}  // namespace JsonHelpers

#endif  // JSON_HELPERS_HPP