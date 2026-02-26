#include "ws/websocket_client.hpp"

WebSocketClient::WebSocketClient(QObject* parent)
    : QObject(parent), reconnectTimer(new QTimer(this)), retryCount(0) {
  connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
  connect(&socket, &QWebSocket::disconnected, this,
          &WebSocketClient::onDisconnected);
  connect(&socket, &QWebSocket::textMessageReceived, this,
          &WebSocketClient::onTextMessageReceived);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  connect(&socket, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
#else
  connect(&socket,
          static_cast<void (QWebSocket::*)(QAbstractSocket::SocketError)>(
              &QWebSocket::error),
          this, &WebSocketClient::onError);
#endif

  // Set up reconnect timer
  reconnectTimer->setSingleShot(true);
  connect(reconnectTimer, &QTimer::timeout, this,
          &WebSocketClient::onReconnectTimer);
}

WebSocketClient::~WebSocketClient() {
  if (reconnectTimer) {
    reconnectTimer->stop();
  }
  socket.close();
}

void WebSocketClient::connectWithToken(const QString& token) {
  authToken = token;
  qInfo().noquote() << "[WS][CONNECT] url=" << ServerConfig::webSocketUrl()
                    << " token_len=" << authToken.size();
  socket.open(QUrl(ServerConfig::webSocketUrl()));
}

void WebSocketClient::disconnectFromServer() { socket.close(); }

bool WebSocketClient::isConnected() const {
  return socket.state() == QAbstractSocket::ConnectedState;
}

void WebSocketClient::sendMessage(int64_t channelId, const QString& body) {
  if (!isConnected()) {
    emit errorReceived("NOT_CONNECTED", "WebSocket is not connected.");
    return;
  }

  ClientSend::SendMessageRequest req;
  req.type = WizzMania::MessageType::SEND_MESSAGE;
  req.id_channel = channelId;
  req.body = body.toStdString();

  const QJsonDocument doc(MessageJson::toJson(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=SEND_MESSAGE channel_id=" << channelId
                    << " body_len=" << body.size() << " payload=" << payload;
  socket.sendTextMessage(payload);
}

void WebSocketClient::openChannel(int64_t channelId) {
  if (!isConnected()) {
    emit errorReceived("NOT_CONNECTED", "WebSocket is not connected.");
    return;
  }

  ClientSend::ChannelHistoryRequest req;
  req.type = WizzMania::MessageType::REQUEST_CHANNEL_HISTORY;
  req.id_channel = channelId;
  req.before_id_message = 0;
  req.limit = 50;

  const QJsonDocument doc(MessageJson::toJson(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=REQUEST_CHANNEL_HISTORY channel_id="
                    << channelId << " payload=" << payload;
  socket.sendTextMessage(payload);
}

void WebSocketClient::markAsRead(int64_t channelId, int64_t lastMessageId) {
  if (!isConnected()) {
    return;
  }

  ClientSend::MarkAsRead req;
  req.type = WizzMania::MessageType::MARK_AS_READ;
  req.id_channel = channelId;
  req.last_id_message = lastMessageId;

  const QJsonDocument doc(MessageJson::toJson(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=MARK_AS_READ channel_id=" << channelId
                    << " last_id_message=" << lastMessageId;

  socket.sendTextMessage(payload);
}

void WebSocketClient::onConnected() {
  resetReconnectState();
  qInfo() << "[WS][CONNECTED]";
  emit connected();
  sendAuth();
}

void WebSocketClient::onDisconnected() {
  qInfo() << "[WS][DISCONNECTED]";
  emit disconnected("Disconnected from server.");
  // Start reconnect timer if we have a token
  if (!authToken.isEmpty()) {
    startReconnectTimer();
  }
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
  qInfo().noquote() << "[WS][RECV] raw=" << message;
  const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
  if (!doc.isObject()) {
    emit errorReceived("INVALID_JSON", "Received invalid JSON payload.");
    return;
  }

  const QJsonObject obj = doc.object();
  const int type = obj.value("type").toInt(-1);

  if (type == static_cast<int>(WizzMania::MessageType::WS_AUTH_SUCCESS)) {
    AuthMessages::WSAuthResponse resp;
    if (MessageJson::fromJson(obj, resp)) {
      emit authenticated(resp.id_user);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::INITIAL_DATA)) {
    ServerSend::InitialDataResponse data;
    if (MessageJson::fromJson(obj, data)) {
      emit initialDataReceived(data);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::CHANNEL_HISTORY)) {
    ServerSend::ChannelHistoryResponse history;
    if (MessageJson::fromJson(obj, history)) {
      emit channelHistoryReceived(history);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::NEW_MESSAGE)) {
    qInfo().noquote() << "[WS][NEW_MESSAGE] type=NEW_MESSAGE channel_id="
                      << obj.value("id_channel").toInt();
    ServerSend::SendMessageResponse msg;
    if (MessageJson::fromJson(obj, msg)) {
      qInfo() << "[WS][NEW_MESSAGE] parsed ok, emitting signal";
      emit newMessageReceived(msg);
      return;
    } else {
      qInfo() << "[WS][NEW_MESSAGE] PARSE_FAILED";
    }
  }
  // user receive new incoming invitation
  if (type == static_cast<int>(WizzMania::MessageType::CHANNEL_INVITATION)) {
    qInfo().noquote()
        << "[WS][CHANNEL_INVITATION] type=CHANNEL_INVITATION channel_id="
        << obj.value("id_channel").toInt();
    ServerSend::ChannelInvitation invit;
    if (MessageJson::fromJson(obj, invit)) {
      qInfo() << "[WS][CHANNEL_INVITATION] parsed ok, emitting signal";
      emit newChannelInvitation(invit);
      return;
    } else {
      qInfo() << "[WS][CHANNEL_INVITATION] PARSE_FAILED";
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::INVITATION_ACCEPTED)) {
    qInfo().noquote()
        << "[WS][INVITATION_ACCEPTED] type=INVITATION_ACCEPTED channel_id="
        << obj.value("id_channel").toInt();
    ServerSend::AcceptInvitationResponse invit;
    if (MessageJson::fromJson(obj, invit)) {
      qInfo() << "[WS][INVITATION_ACCEPTED] parsed ok, emitting signal";
      emit newInvitationAccepted(invit);
      return;
    } else {
      qInfo() << "[WS][INVITATION_ACCEPTED] PARSE_FAILED";
    }
  }


  if (type == static_cast<int>(WizzMania::MessageType::INVITATION_REJECTED)) {
    qInfo().noquote()
        << "[WS][INVITATION_REJECTED] type=INVITATION_REJECTED channel_id="
        << obj.value("id_channel").toInt();
    ServerSend::RejectInvitationResponse rejection;
    if (MessageJson::fromJson(obj, rejection)) {
      qInfo() << "[WS][INVITATION_REJECTED] parsed ok, emitting signal";
      emit newInvitationRejected(rejection);
      return;
    } else {
      qInfo() << "[WS][INVITATION_REJECTED] PARSE_FAILED";
    }
  }


  if (type == static_cast<int>(WizzMania::MessageType::USER_JOINED)) {
    qInfo().noquote()
        << "[WS][USER_JOINED] channel_id=" << obj.value("id_channel").toInt()
        << " user_id="
        << obj.value("contact").toObject().value("id_user").toInt();
    ServerSend::UserJoinedNotification notification;
    if (MessageJson::fromJson(obj, notification)) {
      qInfo() << "[WS][USER_JOINED] parsed ok, emitting signal";
      emit userJoinedChannel(notification);
      return;
    } else {
      qInfo() << "[WS][USER_JOINED] PARSE_FAILED";
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::USER_LEFT)) {
    qInfo().noquote() << "[WS][USER_LEFT] channel_id="
                      << obj.value("id_channel").toInt()
                      << " user_id=" << obj.value("id_user").toInt();
    ServerSend::UserLeftNotification notification;
    if (MessageJson::fromJson(obj, notification)) {
      qInfo() << "[WS][USER_LEFT] parsed ok, emitting signal";
      emit userLeftChannel(notification);
      return;
    } else {
      qInfo() << "[WS][USER_LEFT] PARSE_FAILED";
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::ERROR)) {
    ServerSend::ErrorResponse err;
    if (MessageJson::fromJson(obj, err)) {
      emit errorReceived(QString::fromStdString(err.error_code),
                         QString::fromStdString(err.message));
      return;
    }
  }
  //  use the server's unread count to keep things in sync:
  if (type == static_cast<int>(WizzMania::MessageType::MARK_AS_READ)) {
    ClientSend::MarkAsRead mark;
    if (MessageJson::fromJson(obj, mark)) {
      emit updateChannelUnreadCount(mark.id_channel, mark.unread_count,
                                    mark.last_id_message);
    }
    return;
  }

  // Unknown or unimplemented message type
  emit errorReceived("NOT_IMPLEMENTED",
                     QString("Message type %1 not implemented").arg(type));
}

void WebSocketClient::onError(QAbstractSocket::SocketError error) {
  Q_UNUSED(error);
  qInfo().noquote() << "[WS][ERROR]" << socket.errorString();
  emit errorReceived("WS_ERROR", socket.errorString());
}

void WebSocketClient::onReconnectTimer() {
  if (retryCount < maxRetries) {
    ++retryCount;
    qInfo() << "[WS][RECONNECT] attempt=" << retryCount;
    socket.open(QUrl(ServerConfig::webSocketUrl()));
  } else {
    emit errorReceived("RECONNECT_FAILED",
                       "Failed to reconnect after 5 attempts.");
  }
}

void WebSocketClient::startReconnectTimer() {
  if (retryCount >= maxRetries) {
    return;  // Already exhausted retries
  }

  // Exponential backoff: 1s, 2s, 4s, 8s, 16s
  int delayMs = initialDelayMs * (1 << retryCount);  // 2^retryCount
  reconnectTimer->start(delayMs);
}

void WebSocketClient::resetReconnectState() {
  retryCount = 0;
  if (reconnectTimer->isActive()) {
    reconnectTimer->stop();
  }
}

void WebSocketClient::sendAuth() {
  if (authToken.isEmpty()) {
    emit errorReceived("NO_TOKEN", "Missing auth token for WebSocket auth.");
    return;
  }

  AuthMessages::WSAuthRequest req;
  req.type = WizzMania::MessageType::WS_AUTH;
  req.token = authToken.toStdString();

  const QJsonDocument doc(MessageJson::toJson(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=WS_AUTH token_len=" << authToken.size()
                    << " payload=" << payload;
  socket.sendTextMessage(payload);
}
