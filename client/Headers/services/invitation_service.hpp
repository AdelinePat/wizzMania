#ifndef INVITATIONSERVICE_H
#define INVITATIONSERVICE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

#include "services/api_client.hpp"
#include "utils/message_json.hpp"

class InvitationService : public QObject {
  Q_OBJECT

 public:
  explicit InvitationService(QObject* parent = nullptr);

  void acceptInvitation(int64_t channelId, const QString& token);
  void rejectInvitation(int64_t channelId, const QString& token);
  void leaveChannel(int64_t channelId, const QString& token);

 signals:
  void invitationAccepted(int64_t channelId,
                          const ServerSend::ChannelInfo& channel);
  void invitationRejected(int64_t channelId);
  void channelLeft(int64_t channelId);
  void invitationFailed(int64_t channelId, const QString& action,
                        const QString& message);

 private:
  void sendInvitationAction(const QString& action, int64_t channelId,
                            const QString& token);

  ApiClient api;
};

#endif  // INVITATIONSERVICE_H
