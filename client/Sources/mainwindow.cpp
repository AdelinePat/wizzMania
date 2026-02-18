#include "mainwindow.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      loginWidget(nullptr),
      wsClient(new WebSocketClient(this)),
      channelPanel(nullptr),
      rightPanel(nullptr) {
  ui->setupUi(this);

  // Create and add login widget to the login page
  loginWidget = new LoginWidget(this);
  QVBoxLayout* loginLayout = new QVBoxLayout(ui->loginPage);
  loginLayout->setContentsMargins(0, 0, 0, 0);
  loginLayout->addWidget(loginWidget);

  channelPanel = new ChannelPanelWidget(ui->leftPanel);
  ui->chatsLabel->hide();
  ui->chatGroupsList->hide();
  ui->leftPanelLayout->addWidget(channelPanel);

  rightPanel = new RightPanelWidget(ui->rightPanel);
  ui->chatTitleLabel->hide();
  ui->messagesList->hide();
  ui->messageInput->hide();
  ui->sendButton->hide();
  ui->rightPanelLayout->addWidget(rightPanel);

  // Connect login signal
  connect(loginWidget, &LoginWidget::loginSuccessful, this,
          &MainWindow::onLoginSuccessful);

  // WebSocket signals
  connect(wsClient, &WebSocketClient::authenticated, this,
          &MainWindow::onWsAuthenticated);
  connect(wsClient, &WebSocketClient::initialDataReceived, this,
          &MainWindow::onInitialDataReceived);
  connect(wsClient, &WebSocketClient::channelHistoryReceived, this,
          &MainWindow::onChannelHistoryReceived);
  connect(wsClient, &WebSocketClient::newMessageReceived, this,
          &MainWindow::onNewMessageReceived);
  connect(wsClient, &WebSocketClient::errorReceived, this,
          &MainWindow::onWsError);
  connect(wsClient, &WebSocketClient::disconnected, this,
          &MainWindow::onWsDisconnected);

  // Setup chat view connections
  setupChatView();

  // Start on login page
  ui->stackedWidget->setCurrentIndex(0);
  setChatEnabled(false);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupChatView() {
  connect(channelPanel, &ChannelPanelWidget::channelSelected, this,
          &MainWindow::onChannelSelected);
  connect(rightPanel, &RightPanelWidget::sendRequested, this,
          &MainWindow::onSendMessageRequested);

  // Set initial splitter sizes (250px for left panel, rest for right)
  ui->chatSplitter->setSizes({250, 650});
}

void MainWindow::onLoginSuccessful(const QString& username,
                                   const QString& token) {
  currentUser = username;
  authToken = token;

  qInfo().noquote() << "[UI][LOGIN] username=" << username
                    << " token_len=" << token.size();

  rightPanel->setChatTitle("Connecting to server...");
  setChatEnabled(false);

  wsClient->connectWithToken(token);

  // Switch to chat view
  ui->stackedWidget->setCurrentIndex(1);

  setWindowTitle(QString("WizzMania - %1").arg(username));
}

void MainWindow::onWsAuthenticated(int64_t idUser) {
  currentUserId = idUser;
  qInfo() << "[WS][AUTH_OK] id_user=" << idUser;
  userNamesById.insert(currentUserId, currentUser);
  setChatEnabled(true);
  rightPanel->setChatTitle("Select a chat to start messaging");
}

void MainWindow::onInitialDataReceived(
    const ServerSend::InitialDataResponse& data) {
  qInfo() << "[WS][INIT_DATA] contacts=" << data.contacts.size()
          << " channels=" << data.channels.size()
          << " invitations=" << data.invitations.size();
  cacheKnownUsers(data);
  populateChannels(data.channels);

  if (data.channels.empty()) {
    rightPanel->clearMessages();
    rightPanel->addPlainMessage("No channels available yet.");
  }
}

void MainWindow::onChannelHistoryReceived(
    const ServerSend::ChannelHistoryResponse& history) {
  qInfo() << "[WS][HISTORY] channel_id=" << history.id_channel
          << " count=" << history.messages.size()
          << " has_more=" << history.has_more;
  if (history.id_channel != currentChannelId) {
    return;
  }

  std::vector<ServerSend::Message> ordered = history.messages;
  std::sort(
      ordered.begin(), ordered.end(),
      [](const ServerSend::Message& left, const ServerSend::Message& right) {
        return left.id_message < right.id_message;
      });

  rightPanel->clearMessages();
  for (const auto& msg : ordered) {
    appendMessageToView(history.id_channel, msg);
  }
}

void MainWindow::onNewMessageReceived(
    const ServerSend::NewMessageBroadcast& msg) {
  qInfo().noquote() << "[WS][NEW_MESSAGE] channel_id=" << msg.id_channel
                    << " sender=" << msg.message.id_sender
                    << " id_message=" << msg.message.id_message
                    << " body_len="
                    << static_cast<int>(msg.message.body.size());
  if (msg.id_channel != currentChannelId) {
    return;
  }
  appendMessageToView(msg.id_channel, msg.message);
}

void MainWindow::onWsError(const QString& code, const QString& message) {
  QMessageBox::warning(this, tr("WebSocket"), tr("%1: %2").arg(code, message));
}

void MainWindow::onWsDisconnected(const QString& reason) {
  setChatEnabled(false);
  QMessageBox::information(this, tr("WebSocket"), reason);
}

void MainWindow::populateChannels(
    const std::vector<ServerSend::ChannelInfo>& channels) {
  channelTitles.clear();

  channelPanel->setChannels(channels);

  for (const auto& channel : channels) {
    for (const auto& participant : channel.participants) {
      userNamesById.insert(participant.id_user,
                           QString::fromStdString(participant.username));
    }

    const QString title = QString::fromStdString(channel.title);
    channelTitles.insert(channel.id_channel, title);
  }
}

void MainWindow::onChannelSelected(int64_t channelId, const QString& title) {
  currentChannelId = channelId;
  qInfo().noquote() << "[UI][CHANNEL_SELECT] id=" << channelId
                    << " title=" << title;
  rightPanel->setChatTitle(title);

  rightPanel->clearMessages();

  if (channelId <= 0) {
    return;
  }

  rightPanel->addPlainMessage("Loading messages...");
  wsClient->openChannel(channelId);
}

void MainWindow::onSendMessageRequested(const QString& message) {
  if (message.isEmpty()) {
    return;
  }

  if (currentChannelId <= 0) {
    QMessageBox::warning(this, tr("Message"), tr("Select a channel first."));
    return;
  }

  qInfo().noquote() << "[UI][SEND] channel_id=" << currentChannelId
                    << " body_len=" << message.size()
                    << " body=" << message;
  wsClient->sendMessage(currentChannelId, message);
  rightPanel->focusInput();

  // Fallback refresh: ensures UI catches up even if a NEW_MESSAGE event is
  // missed.
  QTimer::singleShot(120, this, [this]() {
    if (currentChannelId > 0 && wsClient && wsClient->isConnected()) {
      wsClient->openChannel(currentChannelId);
    }
  });
}

void MainWindow::cacheKnownUsers(const ServerSend::InitialDataResponse& data) {
  if (!currentUser.isEmpty() && currentUserId > 0) {
    userNamesById.insert(currentUserId, currentUser);
  }

  for (const auto& contact : data.contacts) {
    userNamesById.insert(contact.id_user,
                         QString::fromStdString(contact.username));
  }

  for (const auto& channel : data.channels) {
    for (const auto& participant : channel.participants) {
      userNamesById.insert(participant.id_user,
                           QString::fromStdString(participant.username));
    }
  }

  userNamesById.insert(1, "System");
}

QString MainWindow::usernameForUserId(int64_t userId) const {
  const auto it = userNamesById.constFind(userId);
  if (it != userNamesById.constEnd()) {
    return it.value();
  }
  return QString("User %1").arg(userId);
}

QWidget* MainWindow::createMessageWidget(const ServerSend::Message& msg) const {
  return new MessageItemWidget(msg, currentUserId,
                               usernameForUserId(msg.id_sender));
}

void MainWindow::appendMessageToView(int64_t channelId,
                                     const ServerSend::Message& msg) {
  Q_UNUSED(channelId);

  rightPanel->addMessageWidget(createMessageWidget(msg));
}

void MainWindow::setChatEnabled(bool enabled) {
  rightPanel->setInputEnabled(enabled);
}
