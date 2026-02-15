#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QHash>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QVBoxLayout>
#include <cstdint>

#include "loginwidget.hpp"
#include "message_structure.hpp"
#include "websocket_client.hpp"

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
  void onNewMessageReceived(const ServerSend::NewMessageBroadcast& msg);
  void onWsError(const QString& code, const QString& message);
  void onWsDisconnected(const QString& reason);
  void onChatSelected(int row);
  void onSendMessage();

 private:
  void setupChatView();
  void addDemoChats();
  void populateChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  void appendMessageToView(int64_t channelId, const ServerSend::Message& msg);
  void setChatEnabled(bool enabled);

  Ui::MainWindow* ui;
  LoginWidget* loginWidget;
  WebSocketClient* wsClient;
  QString currentUser;
  QString authToken;
  int64_t currentUserId = -1;
  int64_t currentChannelId = -1;
  QHash<int64_t, QString> channelTitles;
};

#endif  // MAINWINDOW_H
