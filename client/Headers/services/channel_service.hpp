#ifndef CHANNELSERVICE_H
#define CHANNELSERVICE_H

#include <QObject>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "message_structure.hpp"
#include "services/api_client.hpp"

class ChannelService : public QObject {
  Q_OBJECT

 public:
  explicit ChannelService(QObject* parent = nullptr);

  void fetchHistory(int64_t channelId, int64_t beforeIdMessage,
                    int limit, const QString& token);

 signals:
  void historyReceived(const ServerSend::ChannelHistoryResponse& history);
  void historyFailed(int64_t channelId, const QString& message);

 private:
  ApiClient api;
};

#endif  // CHANNELSERVICE_H