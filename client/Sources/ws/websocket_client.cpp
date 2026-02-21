#include "ws/websocket_client.hpp"

#include <QTimer>

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

  const QJsonDocument doc(MessageJson::to_json(req));
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

  const QJsonDocument doc(MessageJson::to_json(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=REQUEST_CHANNEL_HISTORY channel_id="
                    << channelId << " payload=" << payload;
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
    if (MessageJson::from_json(obj, resp)) {
      emit authenticated(resp.id_user);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::INITIAL_DATA)) {
    ServerSend::InitialDataResponse data;
    if (MessageJson::from_json(obj, data)) {
      emit initialDataReceived(data);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::CHANNEL_HISTORY)) {
    ServerSend::ChannelHistoryResponse history;
    if (MessageJson::from_json(obj, history)) {
      emit channelHistoryReceived(history);
      return;
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::NEW_MESSAGE)) {
    qInfo().noquote() << "[WS][NEW_MESSAGE] type=NEW_MESSAGE channel_id="
                      << obj.value("id_channel").toInt();
    ServerSend::SendMessageResponse msg;
    if (MessageJson::from_json(obj, msg)) {
      qInfo() << "[WS][NEW_MESSAGE] parsed ok, emitting signal";
      emit newMessageReceived(msg);
      return;
    } else {
      qInfo() << "[WS][NEW_MESSAGE] PARSE_FAILED";
    }
  }

  if (type == static_cast<int>(WizzMania::MessageType::ERROR)) {
    ServerSend::ErrorResponse err;
    if (MessageJson::from_json(obj, err)) {
      emit errorReceived(QString::fromStdString(err.error_code),
                         QString::fromStdString(err.message));
      return;
    }
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

  const QJsonDocument doc(MessageJson::to_json(req));
  const QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
  qInfo().noquote() << "[WS][SEND] type=WS_AUTH token_len=" << authToken.size()
                    << " payload=" << payload;
  socket.sendTextMessage(payload);
}