#include "utils/message_json.hpp"

namespace MessageJson {
namespace {
int type_to_int(WizzMania::MessageType type) { return static_cast<int>(type); }

bool parse_message(const QJsonObject& obj, ServerSend::Message& out) {
  if (!obj.contains("id_message") || !obj.contains("id_sender") ||
      !obj.contains("body") || !obj.contains("timestamp")) {
    return false;
  }
  out.id_message = obj.value("id_message").toVariant().toLongLong();
  out.id_sender = obj.value("id_sender").toVariant().toLongLong();
  out.body = obj.value("body").toString().toStdString();
  out.timestamp = obj.value("timestamp").toString().toStdString();
  out.is_system = obj.value("is_system").toBool(false);
  return true;
}

bool parse_contact(const QJsonObject& obj, ServerSend::Contact& out) {
  if (!obj.contains("id_user") || !obj.contains("username")) {
    return false;
  }
  out.id_user = obj.value("id_user").toVariant().toLongLong();
  out.username = obj.value("username").toString().toStdString();
  return true;
}

bool parse_channel(const QJsonObject& obj, ServerSend::ChannelInfo& out) {
  if (!obj.contains("id_channel") || !obj.contains("title")) {
    return false;
  }
  out.id_channel = obj.value("id_channel").toVariant().toLongLong();
  out.title = obj.value("title").toString().toStdString();
  out.is_group = obj.value("is_group").toBool(false);
  out.created_by = obj.value("created_by").toVariant().toLongLong();
  out.unread_count = obj.value("unread_count").toVariant().toLongLong();
  out.last_read_id_message =
      obj.value("last_read_id_message").toVariant().toLongLong();

  if (obj.contains("last_message") && obj.value("last_message").isObject()) {
    parse_message(obj.value("last_message").toObject(), out.last_message);
  }

  if (obj.contains("participants") && obj.value("participants").isArray()) {
    const QJsonArray arr = obj.value("participants").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) continue;
      ServerSend::Contact contact;
      if (parse_contact(val.toObject(), contact)) {
        out.participants.push_back(contact);
      }
    }
  }

  return true;
}

bool parse_invitation(const QJsonObject& obj,
                      ServerSend::ChannelInvitation& out) {
  if (!obj.contains("id_channel") || !obj.contains("title") ||
      !obj.contains("id_inviter")) {
    return false;
  }
  out.id_channel = obj.value("id_channel").toVariant().toLongLong();
  out.title = obj.value("title").toString().toStdString();
  out.id_inviter = obj.value("id_inviter").toVariant().toLongLong();

  if (obj.contains("other_participant_ids") &&
      obj.value("other_participant_ids").isArray()) {
    const QJsonArray arr = obj.value("other_participant_ids").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) continue;
      ServerSend::Contact contact;
      if (parse_contact(val.toObject(), contact)) {
        out.other_participant_ids.push_back(contact);
      }
    }
  }

  return true;
}
}  // namespace

QJsonObject toJson(const AuthMessages::WSAuthRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["token"] = QString::fromStdString(req.token);
  return obj;
}

QJsonObject toJson(const ClientSend::SendMessageRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  obj["body"] = QString::fromStdString(req.body);
  return obj;
}

QJsonObject toJson(const ClientSend::ChannelOpenRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  return obj;
}

QJsonObject toJson(const ClientSend::ChannelHistoryRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  obj["before_id_message"] = static_cast<qint64>(req.before_id_message);
  obj["limit"] = req.limit;
  return obj;
}

QJsonObject toJson(const ClientSend::MarkAsRead& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  obj["last_id_message"] = static_cast<qint64>(req.last_id_message);
  return obj;
}

QJsonObject toJson(const ClientSend::AcceptInvitationRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  return obj;
}

QJsonObject toJson(const ClientSend::RejectInvitationRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  return obj;
}

QJsonObject toJson(const ClientSend::LeaveChannelRequest& req) {
  QJsonObject obj;
  obj["type"] = type_to_int(req.type);
  obj["id_channel"] = static_cast<qint64>(req.id_channel);
  return obj;
}

bool fromJson(const QJsonObject& obj, AuthMessages::WSAuthResponse& out) {
  if (!obj.contains("type")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::WS_AUTH_SUCCESS)) {
    return false;
  }
  out.type = WizzMania::MessageType::WS_AUTH_SUCCESS;
  out.message = obj.value("message").toString().toStdString();
  out.id_user = obj.value("id_user").toVariant().toLongLong();
  return true;
}

bool fromJson(const QJsonObject& obj, ServerSend::SendMessageResponse& out) {
  if (!obj.contains("type") || !obj.contains("id_channel") ||
      !obj.contains("message")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::NEW_MESSAGE)) {
    return false;
  }
  out.type = WizzMania::MessageType::NEW_MESSAGE;
  out.id_channel = obj.value("id_channel").toVariant().toLongLong();
  if (!obj.value("message").isObject()) {
    return false;
  }
  return parse_message(obj.value("message").toObject(), out.message);
}

bool fromJson(const QJsonObject& obj, ServerSend::InitialDataResponse& out) {
  if (!obj.contains("type")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::INITIAL_DATA)) {
    return false;
  }
  out.type = WizzMania::MessageType::INITIAL_DATA;

  out.contacts.clear();
  out.channels.clear();
  out.invitations.clear();
  out.outgoing_invitations.clear();

  if (obj.contains("contacts") && obj.value("contacts").isArray()) {
    const QJsonArray arr = obj.value("contacts").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) {
        continue;
      }
      ServerSend::Contact contact;
      if (parse_contact(val.toObject(), contact)) {
        out.contacts.push_back(contact);
      }
    }
  }

  if (obj.contains("channels") && obj.value("channels").isArray()) {
    const QJsonArray arr = obj.value("channels").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) {
        continue;
      }
      ServerSend::ChannelInfo channel;
      if (parse_channel(val.toObject(), channel)) {
        out.channels.push_back(channel);
      }
    }
  }

  if (obj.contains("invitations") && obj.value("invitations").isArray()) {
    const QJsonArray arr = obj.value("invitations").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) {
        continue;
      }
      ServerSend::ChannelInvitation inv;
      if (parse_invitation(val.toObject(), inv)) {
        out.invitations.push_back(inv);
      }
    }
  }

  if (obj.contains("outgoing_invitations") &&
      obj.value("outgoing_invitations").isArray()) {
    const QJsonArray arr = obj.value("outgoing_invitations").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) {
        continue;
      }
      ServerSend::ChannelInfo channel;
      if (parse_channel(val.toObject(), channel)) {
        out.outgoing_invitations.push_back(channel);
      }
    }
  }

  return true;
}

bool fromJson(const QJsonObject& obj, ServerSend::ChannelHistoryResponse& out) {
  if (!obj.contains("type") || !obj.contains("id_channel")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::CHANNEL_HISTORY)) {
    return false;
  }
  out.type = WizzMania::MessageType::CHANNEL_HISTORY;
  out.id_channel = obj.value("id_channel").toVariant().toLongLong();
  out.has_more = obj.value("has_more").toBool(false);

  out.messages.clear();
  if (obj.contains("messages") && obj.value("messages").isArray()) {
    const QJsonArray arr = obj.value("messages").toArray();
    for (const QJsonValue& val : arr) {
      if (!val.isObject()) {
        continue;
      }
      ServerSend::Message msg;
      if (parse_message(val.toObject(), msg)) {
        out.messages.push_back(msg);
      }
    }
  }

  return true;
}

bool fromJson(const QJsonObject& obj, ServerSend::ErrorResponse& out) {
  if (!obj.contains("type") || !obj.contains("error_code") ||
      !obj.contains("message")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::ERROR)) {
    return false;
  }
  out.type = WizzMania::MessageType::ERROR;
  out.error_code = obj.value("error_code").toString().toStdString();
  out.message = obj.value("message").toString().toStdString();
  return true;
}

bool fromJson(const QJsonObject& obj, ClientSend::MarkAsRead& mark) {
  if (!obj.contains("type") || !obj.contains("unread_count") ||
      !obj.contains("id_channel") || !obj.contains("last_id_message")) {
    return false;
  }
  const int type = obj.value("type").toInt();
  if (type != type_to_int(WizzMania::MessageType::MARK_AS_READ)) {
    return false;
  }
  mark.type = WizzMania::MessageType::MARK_AS_READ;
  mark.id_channel = obj.value("id_channel").toVariant().toLongLong();
  mark.unread_count = obj.value("unread_count").toVariant().toInt();
  mark.last_id_message = obj.value("last_id_message").toVariant().toLongLong();

  return true;
}

bool fromJson(const QJsonObject& obj,
              ServerSend::AcceptInvitationResponse& out) {
  if (!obj.contains("channel") || !obj.value("channel").isObject()) {
    return false;
  }
  return parse_channel(obj.value("channel").toObject(), out.channel);
}

bool fromJson(const QJsonObject& obj, ServerSend::ChannelInvitation& invit) {
  if (!obj.contains("id_channel") || !obj.contains("id_inviter") ||
      !obj.contains("other_participant_ids") || !obj.contains("title")) {
    return false;
  }
  return parse_invitation(obj, invit);
}
}  // namespace MessageJson

// struct ChannelInvitation {
//   WizzMania::MessageType type;  // CHANNEL_INVITATION
//   int64_t id_channel;
//   int64_t id_inviter;
//   std::vector<Contact> other_participant_ids;
//   std::string title;
// };n