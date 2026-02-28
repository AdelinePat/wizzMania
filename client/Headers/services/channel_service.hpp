#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QStringList>

#include "message_structure.hpp"
#include "services/api_client.hpp"
#include "utils/message_json.hpp"

class ChannelService : public QObject {
  Q_OBJECT

 public:
  explicit ChannelService(QObject* parent = nullptr);

  void fetchHistory(int64_t channelId, int64_t beforeIdMessage, int limit,
                    const QString& token);
  void createChannel(const QStringList& usernames, const QString& title,
                     const QString& token);

 signals:
  void historyReceived(const ServerSend::ChannelHistoryResponse& history);
  void historyFailed(int64_t channelId, const QString& message);
  void channelCreated(const ServerSend::CreateChannelResponse& response);
  void createChannelFailed(const QString& message);

 private:
  ApiClient api;
};

#endif  // CHANNELSERVICE_H