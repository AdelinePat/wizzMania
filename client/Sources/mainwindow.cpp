#include "mainwindow.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      loginWidget(nullptr),
      wsClient(new WebSocketClient(this)),
      channelService(new ChannelService(this)),
      invitationService(new InvitationService(this)),
      channelPanel(nullptr),
      rightPanel(nullptr),
      incomingInvitationModel(new IncomingInvitationModel(this)),
      outgoingInvitationModel(new OutgoingInvitationModel(this)) {
  ui->setupUi(this);

  // Create and add login widget to the login page
  loginWidget = new LoginWidget(this);
  QVBoxLayout* loginLayout = new QVBoxLayout(ui->loginPage);
  loginLayout->setContentsMargins(0, 0, 0, 0);
  loginLayout->addWidget(loginWidget);

  channelPanel = new ChannelPanelWidget(ui->leftPanel);
  ui->appLabel->hide();
  ui->chatGroupsList->hide();
  ui->leftPanelLayout->addWidget(channelPanel);

  rightPanel = new RightPanelWidget(ui->rightPanel);
  ui->chatTitleLabel->hide();
  ui->messagesList->hide();
  ui->messageInput->hide();
  ui->sendButton->hide();
  ui->rightPanelLayout->addWidget(rightPanel);

  // User home widget (hidden by default)
  userHomeWidget = new UserHomeWidget(ui->rightPanel);
  ui->rightPanelLayout->addWidget(userHomeWidget);
  userHomeWidget->setModels(incomingInvitationModel, outgoingInvitationModel);
  userHomeWidget->hide();

  // Connect login signal
  connect(loginWidget, &LoginWidget::loginSuccessful, this,
          &MainWindow::onLoginSuccessful);

  // WebSocket signals
  connect(wsClient, &WebSocketClient::authenticated, this,
          &MainWindow::onWsAuthenticated);
  connect(wsClient, &WebSocketClient::initialDataReceived, this,
          &MainWindow::onInitialDataReceived);
  // connect(wsClient, &WebSocketClient::channelHistoryReceived, this,
  //         &MainWindow::onChannelHistoryReceived);
  connect(channelService, &ChannelService::historyReceived, this,
          &MainWindow::onChannelHistoryReceived);

  connect(channelService, &ChannelService::historyFailed, this,
          &MainWindow::onHistoryFailed);

  connect(wsClient, &WebSocketClient::newMessageReceived, this,
          &MainWindow::onNewMessageReceived);
  // signal for update channel unread count
  connect(wsClient, &WebSocketClient::updateChannelUnreadCount, this,
          &MainWindow::onUpdateChannelUnreadCount);

  connect(wsClient, &WebSocketClient::errorReceived, this,
          &MainWindow::onWsError);
  connect(wsClient, &WebSocketClient::disconnected, this,
          &MainWindow::onWsDisconnected);

  // Channel panel portrait click -> show user home
  connect(channelPanel, &ChannelPanelWidget::userHomeRequested, this, [this]() {
    if (userHomeWidget) {
      rightPanel->hide();
      userHomeWidget->show();
    }
  });

  // Forward invitation actions to HTTP API (not WebSocket)
  connect(userHomeWidget, &UserHomeWidget::acceptInvitationRequested, this,
          [this](int64_t id) { acceptInvitation(id); });
  connect(userHomeWidget, &UserHomeWidget::rejectInvitationRequested, this,
          [this](int64_t id) { rejectInvitation(id); });
  connect(userHomeWidget, &UserHomeWidget::cancelInvitationRequested, this,
          [this](int64_t id) {
            if (invitationService)
              invitationService->leaveChannel(id, authToken);
          });

  connect(invitationService, &InvitationService::invitationAccepted, this,
          [this](int64_t id) {
            if (incomingInvitationModel) {
              incomingInvitationModel->removeInvitation(id);
            }
          });
  connect(invitationService, &InvitationService::invitationRejected, this,
          [this](int64_t id) {
            if (incomingInvitationModel) {
              incomingInvitationModel->removeInvitation(id);
            }
          });
  connect(invitationService, &InvitationService::channelLeft, this,
          [this](int64_t id) {
            if (outgoingInvitationModel) {
              outgoingInvitationModel->removeInvitation(id);
            }
          });
  connect(invitationService, &InvitationService::invitationFailed, this,
          [this](int64_t id, const QString& action, const QString& message) {
            Q_UNUSED(id);
            QMessageBox::warning(
                this, tr("Invitation"),
                tr("Failed to %1 invitation: %2").arg(action, message));
          });

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

  // If no channel is currently selected, show the user home by default
  // so the user sees invitations and profile info immediately after login.
  if (currentChannelId <= 0 && userHomeWidget) {
    rightPanel->hide();
    userHomeWidget->show();
  }
}

void MainWindow::onInitialDataReceived(
    const ServerSend::InitialDataResponse& data) {
  qInfo() << "[WS][INIT_DATA] contacts=" << data.contacts.size()
          << " channels=" << data.channels.size()
          << " invitations=" << data.invitations.size();
  cacheKnownUsers(data);
  populateChannels(data.channels);
  // Populate user home invitations via models
  if (userHomeWidget && incomingInvitationModel && outgoingInvitationModel) {
    userHomeWidget->setUsernameCache(&userNamesById);
    incomingInvitationModel->setInvitations(data.invitations);
    outgoingInvitationModel->setInvitations(data.outgoing_invitations);
  }

  if (data.channels.empty()) {
    rightPanel->clearMessages();
    rightPanel->addPlainMessage("No channels available yet.");
  }
}

void MainWindow::onUpdateChannelUnreadCount(int64_t id_channel, int count,
                                            int64_t last_id_message) {
  channelPanel->updateChannelUnreadCount(id_channel, count, last_id_message);
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

  // Mark messages as read and clear unread badge
  int64_t lastMessageId = 0;
  if (!ordered.empty()) {
    lastMessageId = ordered.back().id_message;
    wsClient->markAsRead(history.id_channel, lastMessageId);
  }

  // Clear unread count when channel is accessed
  if (channelPanel) {
    channelPanel->updateChannelUnreadCount(
        history.id_channel, 0,
        lastMessageId);  // TODO CHECK HOW TO GET REAL DATA INSTEAD OF 0!
  }
}

void MainWindow::onNewMessageReceived(
    const ServerSend::SendMessageResponse& msg) {
  qInfo().noquote() << "[WS][NEW_MESSAGE] channel_id=" << msg.id_channel
                    << " sender=" << msg.message.id_sender
                    << " id_message=" << msg.message.id_message << " body_len="
                    << static_cast<int>(msg.message.body.size());
  // Update channel preview / unread badge regardless of active channel
  QString preview = QString::fromStdString(msg.message.body);
  bool incrementUnread = (msg.id_channel != currentChannelId &&
                          msg.message.id_sender != currentUserId);
  if (channelPanel) {
    channelPanel->updateChannelOnNewMessage(msg.id_channel, preview,
                                            incrementUnread);
  }

  // If the message is for the currently open channel, append to view
  if (msg.id_channel == currentChannelId) {
    appendMessageToView(msg.id_channel, msg.message);
  }
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
  // When a channel is selected, ensure the right panel (messages) is shown
  // and the user home view is hidden.
  if (userHomeWidget) {
    userHomeWidget->hide();
  }
  if (rightPanel) {
    rightPanel->show();
  }
  rightPanel->setChatTitle(title);

  rightPanel->clearMessages();

  if (channelId <= 0) {
    return;
  }

  rightPanel->addPlainMessage("Loading messages...");
  // wsClient->openChannel(channelId);
  channelService->fetchHistory(channelId, 0, 50, authToken);
}

void MainWindow::onSendMessageRequested(const QString& message) {
  //  Probably redundant
  if (message.isEmpty()) {
    return;
  }

  if (currentChannelId <= 0) {
    QMessageBox::warning(this, tr("Message"), tr("Select a channel first."));
    return;
  }

  qInfo().noquote() << "[UI][SEND] channel_id=" << currentChannelId
                    << " body_len=" << message.size() << " body=" << message;
  wsClient->sendMessage(currentChannelId, message);
  rightPanel->focusInput();

  // Fallback refresh: ensures UI catches up even if a NEW_MESSAGE event is
  // missed.
  // QTimer::singleShot(120, this, [this]() {
  //   if (currentChannelId > 0 && wsClient && wsClient->isConnected()) {
  //     wsClient->openChannel(currentChannelId);
  //   }
  // });
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

QString MainWindow::resolveAtMentions(const QString& text) const {
  QString result = text;
  // Replace @ID with @username using regex
  QRegularExpression re("@(\\d+)");
  QRegularExpressionMatchIterator it = re.globalMatch(text);

  // Process matches in reverse order to avoid offset issues
  QList<QRegularExpressionMatch> matches;
  while (it.hasNext()) {
    matches.prepend(it.next());
  }

  for (const auto& match : matches) {
    bool ok;
    int64_t userId = match.captured(1).toLongLong(&ok);
    if (ok) {
      QString username = usernameForUserId(userId);
      result.replace(match.capturedStart(), match.capturedLength(),
                     "@" + username);
    }
  }

  return result;
}

QWidget* MainWindow::createMessageWidget(const ServerSend::Message& msg) const {
  QString resolvedBody;
  if (msg.is_system) {
    resolvedBody = resolveAtMentions(QString::fromStdString(msg.body));
  }
  return new MessageItemWidget(msg, currentUserId,
                               usernameForUserId(msg.id_sender), resolvedBody);
}

void MainWindow::appendMessageToView(int64_t channelId,
                                     const ServerSend::Message& msg) {
  Q_UNUSED(channelId);

  rightPanel->addMessageWidget(createMessageWidget(msg));
}

void MainWindow::setChatEnabled(bool enabled) {
  rightPanel->setInputEnabled(enabled);
}

void MainWindow::onHistoryFailed(int64_t channelId, const QString& message) {
  Q_UNUSED(channelId);
  rightPanel->clearMessages();
  rightPanel->addPlainMessage("Failed to load messages: " + message);
}

void MainWindow::acceptInvitation(int64_t id_channel) {
  qInfo() << "[HTTP] ACCEPT_INVITATION channel_id=" << id_channel;
  if (invitationService) {
    invitationService->acceptInvitation(id_channel, authToken);
  }
}

void MainWindow::rejectInvitation(int64_t id_channel) {
  qInfo() << "[HTTP] REJECT_INVITATION channel_id=" << id_channel;
  if (invitationService) {
    invitationService->rejectInvitation(id_channel, authToken);
  }
}