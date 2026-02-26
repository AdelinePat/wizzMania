#include "services/channel_service.hpp"

ChannelService::ChannelService(QObject* parent) : QObject(parent) {}

void ChannelService::createChannel(const QStringList& usernames,
                                   const QString& title, const QString& token) {
  ClientSend::CreateChannelRequest request;
  request.type = WizzMania::MessageType::CREATE_CHANNEL;
  request.title = title.toStdString();
  for (const QString& username : usernames) {
    request.usernames.insert(username.toStdString());
  }

  const QJsonObject payload = MessageJson::toJson(request);

  QNetworkReply* reply = api.postJsonAuth("channels", payload, token);

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();

    const QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
      emit createChannelFailed("Invalid JSON response from server.");
      reply->deleteLater();
      return;
    }

    const QJsonObject obj = doc.object();

    if (reply->error() != QNetworkReply::NoError && statusCode != 409) {
      const QString message = obj.value("message").toString();
      emit createChannelFailed(message.isEmpty() ? reply->errorString()
                                                 : message);
      reply->deleteLater();
      return;
    }

    ServerSend::CreateChannelResponse response;
    response.type = WizzMania::MessageType::CHANNEL_CREATED;
    response.id_channel = obj.value("id_channel").toVariant().toLongLong();
    response.already_existed = obj.value("already_existed").toBool(false);

    if (statusCode == 409 || response.already_existed) {
      response.already_existed = true;
      emit channelCreated(response);
      reply->deleteLater();
      return;
    }

    if (!MessageJson::fromJson(obj, response)) {
      emit createChannelFailed("Failed to parse create channel response.");
      reply->deleteLater();
      return;
    }

    emit channelCreated(response);
    reply->deleteLater();
  });
}

void ChannelService::fetchHistory(int64_t channelId, int64_t beforeIdMessage,
                                  int limit, const QString& token) {
  // Build path with query params
  // e.g. /channels/42/history?before_id_message=0&limit=50
  const QString path =
      QString("channels/%1/history?before_id_message=%2&limit=%3")
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
      emit historyFailed(channelId,
                         message.isEmpty() ? reply->errorString() : message);
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