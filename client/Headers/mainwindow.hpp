#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <cstdint>
#include <QDebug>

#include "login_widget.hpp"
#include "message_structure.hpp"
#include "widgets/channel_panel_widget.hpp"
#include "widgets/message_item_widget.hpp"
#include "widgets/right_panel_widget.hpp"
#include "ws/websocket_client.hpp"

class LoginWidget;
class WebSocketClient;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private slots:
  void onLoginSuccessful(const QString& username, const QString& token);
  void onWsAuthenticated(int64_t idUser);
  void onInitialDataReceived(const ServerSend::InitialDataResponse& data);
  void onChannelHistoryReceived(
      const ServerSend::ChannelHistoryResponse& history);
  void onNewMessageReceived(const ServerSend::NewMessageBroadcast& msg);
  void onWsError(const QString& code, const QString& message);
  void onWsDisconnected(const QString& reason);
  void onChannelSelected(int64_t channelId, const QString& title);
  void onSendMessageRequested(const QString& message);

 private:
  void setupChatView();
  void cacheKnownUsers(const ServerSend::InitialDataResponse& data);
  void populateChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  QString usernameForUserId(int64_t userId) const;
  QWidget* createMessageWidget(const ServerSend::Message& msg) const;
  void appendMessageToView(int64_t channelId, const ServerSend::Message& msg);
  void setChatEnabled(bool enabled);

  Ui::MainWindow* ui;
  LoginWidget* loginWidget;
  WebSocketClient* wsClient;
  ChannelPanelWidget* channelPanel;
  RightPanelWidget* rightPanel;
  QString currentUser;
  QString authToken;
  int64_t currentUserId = -1;
  int64_t currentChannelId = -1;
  QHash<int64_t, QString> channelTitles;
  QHash<int64_t, QString> userNamesById;
};

#endif  // MAINWINDOW_H
