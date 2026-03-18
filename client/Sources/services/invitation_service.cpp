#include "services/invitation_service.hpp"

InvitationService::InvitationService(QObject* parent) : QObject(parent) {}

void InvitationService::acceptInvitation(int64_t channelId,
                                         const QString& token) {
  sendInvitationAction("accept", channelId, token);
}

void InvitationService::rejectInvitation(int64_t channelId,
                                         const QString& token) {
  sendInvitationAction("reject", channelId, token);
}

void InvitationService::cancelInvitation(int64_t channelId,
                                         const QString& token) {
  sendInvitationAction("cancel", channelId, token);
}

void InvitationService::leaveChannel(int64_t channelId, const QString& token) {
  const QString path = QString("channels/%1/leave").arg(channelId);
  QNetworkReply* reply = api.patchAuth(path, token);

  connect(reply, &QNetworkReply::finished, this, [this, reply, channelId]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      emit invitationFailed(channelId, "leave",
                            api.extractErrorMessage(reply, body));
      reply->deleteLater();
      return;
    }

    emit channelLeft(channelId);
    reply->deleteLater();
  });
}

void InvitationService::sendInvitationAction(const QString& action,
                                             int64_t channelId,
                                             const QString& token) {
  const QString path = QString("invitations/%1/%2").arg(channelId).arg(action);
  QNetworkReply* reply = api.patchAuth(path, token);

  connect(
      reply, &QNetworkReply::finished, this,
      [this, reply, channelId, action]() {
        const int statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();

        if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
          emit invitationFailed(channelId, action,
                                api.extractErrorMessage(reply, body));
          reply->deleteLater();
          return;
        }

        if (action == "accept") {
          ServerSend::AcceptInvitationResponse response;
          QJsonDocument doc = QJsonDocument::fromJson(body);
          if (doc.isObject() && MessageJson::fromJson(doc.object(), response)) {
            emit invitationAccepted(channelId, response.channel);
          } else {
            emit invitationFailed(channelId, action,
                                  "Failed to parse channel data");
          }
        } else if (action == "reject") {
          emit invitationRejected(channelId);
        } else if (action == "cancel") {
          emit invitationCanceled(channelId);
        }

        reply->deleteLater();
      });
}
