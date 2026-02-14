#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <QAbstractSocket>
#include <QObject>
#include <QWebSocket>

#include "message_structure.hpp"

class WebSocketClient : public QObject {
  Q_OBJECT

 public:
  explicit WebSocketClient(QObject* parent = nullptr);

  void connectWithToken(const QString& token);
  void disconnectFromServer();
  bool isConnected() const;

  void sendMessage(int64_t channelId, const QString& body);
  void openChannel(int64_t channelId);

 signals:
  void connected();
  void disconnected(const QString& reason);
  void authenticated(int64_t idUser);
  void initialDataReceived(const ServerSend::InitialDataResponse& data);
  void newMessageReceived(const ServerSend::NewMessageBroadcast& msg);
  void errorReceived(const QString& code, const QString& message);

 private slots:
  void onConnected();
  void onDisconnected();
  void onTextMessageReceived(const QString& message);
  void onError(QAbstractSocket::SocketError error);

 private:
  void sendAuth();

  QWebSocket socket;
  QString authToken;
};

#endif  // WEBSOCKET_CLIENT_H
