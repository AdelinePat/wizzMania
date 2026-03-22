#ifndef MESSAGE_JSON_H
#define MESSAGE_JSON_H

#include <QJsonArray>
#include <QJsonObject>

#include "message_structure.hpp"

namespace MessageJson {
QJsonObject toJson(const AuthMessages::WSAuthRequest& req);
QJsonObject toJson(const ClientSend::SendMessageRequest& req);
QJsonObject toJson(const ClientSend::WizzRequest& req);
QJsonObject toJson(const ClientSend::CreateChannelRequest& req);
QJsonObject toJson(const ClientSend::ChannelOpenRequest& req);
QJsonObject toJson(const ClientSend::ChannelHistoryRequest& req);
QJsonObject toJson(const ClientSend::MarkAsRead& req);
QJsonObject toJson(const ClientSend::AcceptInvitationRequest& req);
QJsonObject toJson(const ClientSend::RejectInvitationRequest& req);
QJsonObject toJson(const ClientSend::LeaveChannelRequest& req);
bool fromJson(const QJsonObject& obj, AuthMessages::WSAuthResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::SendMessageResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::InitialDataResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::ChannelHistoryResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::ErrorResponse& out);
bool fromJson(const QJsonObject& obj, ClientSend::MarkAsRead& mark);
bool fromJson(const QJsonObject& obj, ServerSend::WizzNotification& out);
bool fromJson(const QJsonObject& obj,
              ServerSend::AcceptInvitationResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::UserJoinedNotification& out);
bool fromJson(const QJsonObject& obj, ServerSend::ChannelInvitation& invit);
bool fromJson(const QJsonObject& obj, ServerSend::UserLeftNotification& out);
bool fromJson(const QJsonObject& obj,
              ServerSend::RejectInvitationResponse& reject);
bool fromJson(const QJsonObject& obj,
              ServerSend::CreateChannelResponse& channel);
// bool fromJson(const QJsonObject& obj,
//               ServerSend::AcceptInvitationResponse& invit);
}  // namespace MessageJson

#endif  // MESSAGE_JSON_H