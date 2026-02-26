#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <QAbstractSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QWebSocket>

#include "message_structure.hpp"
#include "services/server_config.hpp"
#include "utils/message_json.hpp"

class WebSocketClient : public QObject {
  Q_OBJECT

 public:
  explicit WebSocketClient(QObject* parent = nullptr);
  ~WebSocketClient();

  void connectWithToken(const QString& token);
  void disconnectFromServer();
  bool isConnected() const;

  void sendMessage(int64_t channelId, const QString& body);
  void openChannel(int64_t channelId);
  void markAsRead(int64_t channelId, int64_t lastMessageId);

 signals:
  void connected();
  void disconnected(const QString& reason);
  void authenticated(int64_t idUser);
  void initialDataReceived(const ServerSend::InitialDataResponse& data);
  void channelHistoryReceived(
      const ServerSend::ChannelHistoryResponse& history);
  void newMessageReceived(const ServerSend::SendMessageResponse& msg);
  // signal to update unread count from server ???
  void updateChannelUnreadCount(int64_t id_channel, int unread_count,
                                int64_t last_id_message);

  void errorReceived(const QString& code, const QString& message);
  void newChannelInvitation(ServerSend::ChannelInvitation& invit);
  void newInvitationAccepted(ServerSend::AcceptInvitationResponse& invit);
  void userLeftChannel(const ServerSend::UserLeftNotification& notification);

 private slots:
  void onConnected();
  void onDisconnected();
  void onTextMessageReceived(const QString& message);
  void onError(QAbstractSocket::SocketError error);
  void onReconnectTimer();

 private:
  void sendAuth();
  void startReconnectTimer();
  void resetReconnectState();

  QWebSocket socket;
  QString authToken;

  // Reconnection parameters
  QTimer* reconnectTimer;
  int retryCount;
  static constexpr int maxRetries = 5;
  static constexpr int initialDelayMs = 1000;
};

#endif  // WEBSOCKET_CLIENT_H