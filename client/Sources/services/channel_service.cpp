#include "services/channel_service.hpp"

#include "utils/message_json.hpp"

ChannelService::ChannelService(QObject* parent) : QObject(parent) {}

void ChannelService::fetchHistory(int64_t channelId, int64_t beforeIdMessage,
                                  int limit, const QString& token) {
  // Build path with query params
  // e.g. /channels/42/history?before_id_message=0&limit=50
  const QString path = QString("channels/%1/history?before_id_message=%2&limit=%3")
                           .arg(channelId)
                           .arg(beforeIdMessage)
                           .arg(limit);

  QNetworkReply* reply = api.getAuth(path, token);

  connect(reply, &QNetworkReply::finished, this, [this, reply, channelId]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      const QString message = QString::fromUtf8(body);
      emit historyFailed(channelId, message.isEmpty()
                                        ? reply->errorString()
                                        : message);
      reply->deleteLater();
      return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
      emit historyFailed(channelId, "Invalid JSON response from server.");
      reply->deleteLater();
      return;
    }

    ServerSend::ChannelHistoryResponse history;
    if (!MessageJson::fromJson(doc.object(), history)) {
      emit historyFailed(channelId, "Failed to parse history response.");
      reply->deleteLater();
      return;
    }

    emit historyReceived(history);
    reply->deleteLater();
  });
}