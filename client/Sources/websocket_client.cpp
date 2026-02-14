#include "websocket_client.h"

#include <QJsonDocument>
#include <QUrl>

#include "message_json.h"
#include "serverconfig.h"

WebSocketClient::WebSocketClient(QObject* parent) : QObject(parent) {
  connect(&socket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
  connect(&socket, &QWebSocket::disconnected, this,
          &WebSocketClient::onDisconnected);
  connect(&socket, &QWebSocket::textMessageReceived, this,
          &WebSocketClient::onTextMessageReceived);
  connect(&socket, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
}

void WebSocketClient::connectWithToken(const QString& token) {
  authToken = token;
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
  socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void WebSocketClient::openChannel(int64_t channelId) {
  if (!isConnected()) {
    emit errorReceived("NOT_CONNECTED", "WebSocket is not connected.");
    return;
  }

  ClientSend::ChannelOpenRequest req;
  req.type = WizzMania::MessageType::CHANNEL_OPEN;
  req.id_channel = channelId;

  const QJsonDocument doc(MessageJson::to_json(req));
  socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void WebSocketClient::onConnected() {
  emit connected();
  sendAuth();
}

void WebSocketClient::onDisconnected() {
  emit disconnected("Disconnected from server.");
}

void WebSocketClient::onTextMessageReceived(const QString& message) {
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

  if (type == static_cast<int>(WizzMania::MessageType::NEW_MESSAGE)) {
    ServerSend::NewMessageBroadcast msg;
    if (MessageJson::from_json(obj, msg)) {
      emit newMessageReceived(msg);
      return;
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
}

void WebSocketClient::onError(QAbstractSocket::SocketError error) {
  Q_UNUSED(error);
  emit errorReceived("WS_ERROR", socket.errorString());
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
  socket.sendTextMessage(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}
