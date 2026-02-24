#ifndef MESSAGE_JSON_H
#define MESSAGE_JSON_H

#include <QJsonArray>
#include <QJsonObject>

#include "message_structure.hpp"

namespace MessageJson {
QJsonObject toJson(const AuthMessages::WSAuthRequest& req);
QJsonObject toJson(const ClientSend::SendMessageRequest& req);
QJsonObject toJson(const ClientSend::ChannelOpenRequest& req);
QJsonObject toJson(const ClientSend::ChannelHistoryRequest& req);
QJsonObject toJson(const ClientSend::MarkAsReadRequest& req);
QJsonObject toJson(const ClientSend::AcceptInvitationRequest& req);
QJsonObject toJson(const ClientSend::RejectInvitationRequest& req);
bool fromJson(const QJsonObject& obj, AuthMessages::WSAuthResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::SendMessageResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::InitialDataResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::ChannelHistoryResponse& out);
bool fromJson(const QJsonObject& obj, ServerSend::ErrorResponse& out);
}  // namespace MessageJson

#endif  // MESSAGE_JSON_H