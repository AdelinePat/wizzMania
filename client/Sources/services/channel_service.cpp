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
      emit createChannelFailed(api.extractErrorMessage(reply, body));
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
      emit historyFailed(channelId, api.extractErrorMessage(reply, body));
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

void ChannelService::sendMessage(int64_t channelId, const QString& body,
                                 const QString& token) {
  ClientSend::SendMessageRequest request;
  request.type = WizzMania::MessageType::SEND_MESSAGE;
  request.id_channel = channelId;
  request.body = body.toStdString();

  QNetworkReply* reply =
      api.postJsonAuth(QString("channels/%1/messages").arg(channelId),
                       MessageJson::toJson(request), token);

  connect(reply, &QNetworkReply::finished, this, [this, reply, channelId]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray rawBody = reply->readAll();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      QString message = api.extractErrorMessage(reply, rawBody);
      if (statusCode == 405) {
        message =
            "Server endpoint not available (405). Restart/rebuild the server "
            "so HTTP routes for messages/wizz/read are active.";
      }
      emit messageSendFailed(channelId, message);
      reply->deleteLater();
      return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(rawBody);
    if (!doc.isObject()) {
      emit messageSendFailed(channelId, "Invalid JSON response from server.");
      reply->deleteLater();
      return;
    }

    ServerSend::SendMessageResponse response;
    if (!MessageJson::fromJson(doc.object(), response)) {
      emit messageSendFailed(channelId,
                             "Failed to parse send message response.");
      reply->deleteLater();
      return;
    }

    emit messageSent(response);
    reply->deleteLater();
  });
}

void ChannelService::markAsRead(int64_t channelId, int64_t lastMessageId,
                                const QString& token) {
  ClientSend::MarkAsRead request;
  request.type = WizzMania::MessageType::MARK_AS_READ;
  request.id_channel = channelId;
  request.last_id_message = lastMessageId;

  QNetworkReply* reply =
      api.patchJsonAuth(QString("channels/%1/read").arg(channelId),
                        MessageJson::toJson(request), token);

  connect(reply, &QNetworkReply::finished, this, [this, reply, channelId]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray rawBody = reply->readAll();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      QString message = api.extractErrorMessage(reply, rawBody);
      if (statusCode == 405) {
        message =
            "Server endpoint not available (405). Restart/rebuild the server "
            "so HTTP routes for messages/wizz/read are active.";
      }
      emit markAsReadFailed(channelId, message);
      reply->deleteLater();
      return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(rawBody);
    if (!doc.isObject()) {
      emit markAsReadFailed(channelId, "Invalid JSON response from server.");
      reply->deleteLater();
      return;
    }

    ClientSend::MarkAsRead response;
    if (!MessageJson::fromJson(doc.object(), response)) {
      emit markAsReadFailed(channelId,
                            "Failed to parse mark-as-read response.");
      reply->deleteLater();
      return;
    }

    emit markAsReadUpdated(response.id_channel,
                           static_cast<int>(response.unread_count),
                           response.last_id_message);
    reply->deleteLater();
  });
}

void ChannelService::sendWizz(int64_t channelId, const QString& token) {
  ClientSend::WizzRequest request;
  request.type = WizzMania::MessageType::WIZZ;
  request.id_channel = channelId;

  QNetworkReply* reply =
      api.postJsonAuth(QString("channels/%1/wizz").arg(channelId),
                       MessageJson::toJson(request), token);

  connect(reply, &QNetworkReply::finished, this, [this, reply, channelId]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray rawBody = reply->readAll();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      QString message = api.extractErrorMessage(reply, rawBody);
      if (statusCode == 405) {
        message =
            "Server endpoint not available (405). Restart/rebuild the server "
            "so HTTP routes for messages/wizz/read are active.";
      }
      emit wizzFailed(channelId, message);
      reply->deleteLater();
      return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(rawBody);
    if (!doc.isObject()) {
      emit wizzFailed(channelId, "Invalid JSON response from server.");
      reply->deleteLater();
      return;
    }

    ServerSend::WizzNotification response;
    if (!MessageJson::fromJson(doc.object(), response)) {
      emit wizzFailed(channelId, "Failed to parse wizz response.");
      reply->deleteLater();
      return;
    }

    emit wizzSent(response);
    reply->deleteLater();
  });
}