#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QStringList>

#include "message_structure.hpp"
#include "services/request_service.hpp"
#include "utils/message_json.hpp"

class ChannelService : public QObject {
  Q_OBJECT

 public:
  explicit ChannelService(QObject* parent = nullptr);

  void fetchHistory(int64_t channelId, int64_t beforeIdMessage, int limit,
                    const QString& token);
  void createChannel(const QStringList& usernames, const QString& title,
                     const QString& token);
  void sendMessage(int64_t channelId, const QString& body,
                   const QString& token);
  void markAsRead(int64_t channelId, int64_t lastMessageId,
                  const QString& token);
  void sendWizz(int64_t channelId, const QString& token);

 signals:
  void historyReceived(const ServerSend::ChannelHistoryResponse& history);
  void historyFailed(int64_t channelId, const QString& message);
  void channelCreated(const ServerSend::CreateChannelResponse& response);
  void createChannelFailed(const QString& message);
  void messageSent(const ServerSend::SendMessageResponse& msg);
  void messageSendFailed(int64_t channelId, const QString& message);
  void markAsReadUpdated(int64_t channelId, int unreadCount,
                         int64_t lastMessageId);
  void markAsReadFailed(int64_t channelId, const QString& message);
  void wizzSent(const ServerSend::WizzNotification& notification);
  void wizzFailed(int64_t channelId, const QString& message);

 private:
  RequestService api;
};

#endif  // CHANNELSERVICE_H