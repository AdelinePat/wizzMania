#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QHash>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <cstdint>

#include "message_structure.hpp"
#include "widgets/channel_panel_widget.hpp"
#include "widgets/login_widget.hpp"
#include "widgets/message_item_widget.hpp"
#include "widgets/right_panel_widget.hpp"
#include "widgets/user_home_widget.hpp"
#include "ws/websocket_client.hpp"

namespace Ui {
class MainWindow;
}  // namespace Ui

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
  void onNewMessageReceived(const ServerSend::SendMessageResponse& msg);
  void onWsError(const QString& code, const QString& message);
  void onWsDisconnected(const QString& reason);
  void onChannelSelected(int64_t channelId, const QString& title);
  void onSendMessageRequested(const QString& message);
  void onUpdateChannelUnreadCount(int64_t id_channel, int count, int64_t last_id_message);

 private:
  void setupChatView();
  void cacheKnownUsers(const ServerSend::InitialDataResponse& data);
  void populateChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  QString usernameForUserId(int64_t userId) const;
  QString resolveAtMentions(const QString& text) const;
  QWidget* createMessageWidget(const ServerSend::Message& msg) const;
  void appendMessageToView(int64_t channelId, const ServerSend::Message& msg);
  void setChatEnabled(bool enabled);

  Ui::MainWindow* ui;
  LoginWidget* loginWidget;
  WebSocketClient* wsClient;
  ChannelPanelWidget* channelPanel;
  RightPanelWidget* rightPanel;
  UserHomeWidget* userHomeWidget;
  QString currentUser;
  QString authToken;
  int64_t currentUserId = -1;
  int64_t currentChannelId = -1;
  QHash<int64_t, QString>
      channelTitles;  // TODO struct ChannelInfo has everything (last read
                      // id_message to send to server, last_message_sent
                      // (preview channel list), unread_count_count)
  QHash<int64_t, QString>
      userNamesById;  // TODO use std::vector<Contact> contacts;?
  // initialDataResponse invitations (outgiong & incoming)
  // user_model --> last
};

#endif  // MAINWINDOW_H
