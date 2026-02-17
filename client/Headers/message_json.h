#ifndef MESSAGE_JSON_H
#define MESSAGE_JSON_H

#include <QJsonObject>

#include "message_structure.hpp"

namespace MessageJson {
QJsonObject to_json(const AuthMessages::WSAuthRequest& req);
QJsonObject to_json(const ClientSend::SendMessageRequest& req);
QJsonObject to_json(const ClientSend::ChannelOpenRequest& req);

bool from_json(const QJsonObject& obj, AuthMessages::WSAuthResponse& out);
bool from_json(const QJsonObject& obj, ServerSend::SendMessageResponse& out);
bool from_json(const QJsonObject& obj, ServerSend::InitialDataResponse& out);
bool from_json(const QJsonObject& obj, ServerSend::ErrorResponse& out);
}  // namespace MessageJson

#endif  // MESSAGE_JSON_H
