#include "mainwindow.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      loginWidget(nullptr),
      registerWidget(nullptr),
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

  // Create register widget (hidden by default)
  registerWidget = new RegisterWidget(this);
  loginLayout->addWidget(registerWidget);
  registerWidget->hide();

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
  authManager = new AuthManager(this);
  ui->rightPanelLayout->addWidget(userHomeWidget);
  userHomeWidget->setModels(incomingInvitationModel, outgoingInvitationModel);
  userHomeWidget->hide();

  // incomingInvitationItemWidget = new IncomingInvitationItemWidget()

  // Connect login signal
  connect(loginWidget, &LoginWidget::loginSuccessful, this,
          &MainWindow::onLoginSuccessful);

  // Connect register signals
  connect(loginWidget, &LoginWidget::registerRequested, this,
          &MainWindow::onRegisterRequested);
  connect(registerWidget, &RegisterWidget::registerRequested, this,
          &MainWindow::onRegisterConfirmed);
  connect(registerWidget, &RegisterWidget::cancelRequested, this,
          &MainWindow::onRegisterCancelled);

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

  connect(
      channelService, &ChannelService::channelCreated, this,
      [this](const ServerSend::CreateChannelResponse& response) {
        if (response.already_existed) {
          if (channelPanel) {
            const ServerSend::ChannelInfo* existing =
                channelPanel->getChannelInfo(response.id_channel);
            if (existing) {
              onChannelSelected(response.id_channel,
                                QString::fromStdString(existing->title));
            }
          }
          return;
        }

        if (outgoingInvitationModel) {
          outgoingInvitationModel->addInvitation(response.channel);
        }
        if (userHomeWidget) {
          userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
        }

        channelTitles.insert(response.channel.id_channel,
                             QString::fromStdString(response.channel.title));
        for (const auto& participant : response.channel.participants) {
          userNamesById.insert(participant.id_user,
                               QString::fromStdString(participant.username));
        }
      });

  connect(channelService, &ChannelService::createChannelFailed, this,
          [this](const QString& message) {
            QMessageBox::warning(this, tr("Create Channel"), message);
          });

  connect(wsClient, &WebSocketClient::newMessageReceived, this,
          &MainWindow::onNewMessageReceived);
  connect(wsClient, &WebSocketClient::wizzReceived, this,
          &MainWindow::onWizzReceived);
  // signal for update channel unread count
  connect(wsClient, &WebSocketClient::updateChannelUnreadCount, this,
          &MainWindow::onUpdateChannelUnreadCount);

  connect(wsClient, &WebSocketClient::errorReceived, this,
          &MainWindow::onWsError);
  connect(wsClient, &WebSocketClient::disconnected, this,
          &MainWindow::onWsDisconnected);

  // newChannelInvitation
  connect(wsClient, &WebSocketClient::newInvitationAccepted, this,
          &MainWindow::onNewInvitationAccepted);

  connect(wsClient, &WebSocketClient::newChannelInvitation, this,
          &MainWindow::onNewInvitationReceived);

  connect(wsClient, &WebSocketClient::userJoinedChannel, this,
          &MainWindow::onUserJoinedChannel);

  connect(wsClient, &WebSocketClient::userLeftChannel, this,
          &MainWindow::onUserLeftChannel);

  connect(wsClient, &WebSocketClient::newInvitationRejected, this,
          &MainWindow::onNewInvitationRejected);

  connect(wsClient, &WebSocketClient::newChannelCreated, this,
          &MainWindow::onNewChannelCreated);
  // logout
  connect(channelPanel, &ChannelPanelWidget::logoutRequested, this,
          &MainWindow::onLogoutRequested);
  connect(authManager, &AuthManager::logoutSucceeded, this,
          &MainWindow::onLogoutSucceeded);
  connect(authManager, &AuthManager::registerSucceeded, this,
          [this](const QString& message) {
            registerWidget->hide();
            loginWidget->show();
            loginWidget->setSuccessText(message);
          });
  connect(authManager, &AuthManager::registerFailed, this,
          [this](const QString& message) {
            if (registerWidget) {
              registerWidget->showErrorMessage(message);
            }
          });
  connect(authManager, &AuthManager::deleteAccountSucceeded, this,
          [this](const QString& message) {
            suppressDisconnectPopup = true;
            onLogoutSucceeded();
            if (loginWidget) {
              loginWidget->setSuccessText(message);
            }
          });
  connect(authManager, &AuthManager::deleteAccountFailed, this,
          [this](const QString& message) {
            QMessageBox::warning(this, tr("Delete Account"), message);
          });

  // Channel panel portrait click -> show user home
  connect(channelPanel, &ChannelPanelWidget::userHomeRequested, this, [this]() {
    if (userHomeWidget) {
      rightPanel->hide();
      userHomeWidget->show();
      currentChannelId = -1;
    }
  });

  // Channel panel create channel button
  connect(channelPanel, &ChannelPanelWidget::createChannelRequested, this,
          [this]() {
            auto* dialog = new CreateChannelWidget(this);
            connect(dialog, &CreateChannelWidget::createChannelRequested, this,
                    [this](const QStringList& usernames, const QString& title) {
                      qInfo() << "[UI] CREATE_CHANNEL usernames=" << usernames
                              << " title=" << title;
                      if (channelService) {
                        channelService->createChannel(usernames, title,
                                                      authToken);
                      }
                    });
            dialog->exec();
            dialog->deleteLater();
          });

  // Channel panel logout button (not functional yet)
  connect(channelPanel, &ChannelPanelWidget::logoutRequested, this,
          [this]() { qInfo() << "[UI] Logout requested"; });

  // Channel panel leave channel button
  connect(channelPanel, &ChannelPanelWidget::leaveChannelRequested, this,
          [this](int64_t channelId) {
            if (invitationService) {
              invitationService->leaveChannel(channelId, authToken);
            }
          });

  // Forward invitation actions to HTTP API (not WebSocket)
  connect(userHomeWidget, &UserHomeWidget::acceptInvitationRequested, this,
          [this](int64_t id) { acceptInvitation(id); });
  connect(userHomeWidget, &UserHomeWidget::rejectInvitationRequested, this,
          [this](int64_t id) { rejectInvitation(id); });
  connect(userHomeWidget, &UserHomeWidget::cancelInvitationRequested, this,
          [this](int64_t id) {
            if (invitationService) {
              invitationService->cancelInvitation(id, authToken);
            }
          });

  connect(userHomeWidget, &UserHomeWidget::deleteAccountRequested, this,
          [this]() {
            qInfo() << "[HTTP] DELETE_ACCOUNT requested";
            if (authManager) {
              authManager->deleteAccount(authToken);
            }
          });

  connect(invitationService, &InvitationService::invitationAccepted, this,
          [this](int64_t id, const ServerSend::ChannelInfo& channel) {
            applyInvitationAccepted(id, channel);
          });
  connect(invitationService, &InvitationService::invitationRejected, this,
          [this](int64_t id) {
            if (incomingInvitationModel) {
              incomingInvitationModel->removeInvitation(id);
            }
          });
  connect(
      invitationService, &InvitationService::invitationCanceled, this,
      [this](int64_t id) {
        if (outgoingInvitationModel) {
          outgoingInvitationModel->removeInvitation(id);
          userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
        }
        statusBar()->showMessage(tr("Invitation canceled."), 2500);
      });
  connect(invitationService, &InvitationService::channelLeft, this,
          [this](int64_t channelId) {
            qInfo() << "[HTTP][LEAVE_SUCCESS] Removing channel" << channelId;

            // If currently viewing this channel, switch to home
            if (currentChannelId == channelId) {
              rightPanel->hide();
              if (userHomeWidget) {
                userHomeWidget->show();
              }
              currentChannelId = -1;
            }

            // Remove from channel panel immediately (HTTP success)
            if (channelPanel) {
              channelPanel->removeChannel(channelId);
            }

            // Also remove from outgoing invitations if it was there
            if (outgoingInvitationModel) {
              outgoingInvitationModel->removeInvitation(channelId);
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
  connect(rightPanel, &RightPanelWidget::wizzRequested, this,
          &MainWindow::onWizzRequested);

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

  // Update user info in channel panel
  if (channelPanel && !currentUser.isEmpty()) {
    QString initials = getUserInitials(currentUser);
    channelPanel->setUserInfo(currentUser, initials);
  }

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
    if (wsClient) {
      wsClient->markAsRead(msg.id_channel, msg.message.id_message);
      if (channelPanel) {
        channelPanel->updateChannelUnreadCount(msg.id_channel, 0,
                                               msg.message.id_message);
      }
    }
    appendMessageToView(msg.id_channel, msg.message);
  }
}

void MainWindow::onWizzReceived(
    const ServerSend::WizzNotification& notification) {
  if (notification.id_user == currentUserId) {
    return;
  }

  const QString sender = usernameForUserId(notification.id_user);
  statusBar()->showMessage(tr("⚡ WIZZ from %1").arg(sender), 3000);
  playWizzAnimation();
}

void MainWindow::playWizzAnimation() {
  if (wizzAnimating) {
    return;
  }

  wizzAnimating = true;
  const QPoint originalPos = pos();

  for (int i = 0; i < 20; ++i) {
    QTimer::singleShot(i * 30, this, [this, originalPos, i]() {
      if (i < 19) {
        const int offsetX = QRandomGenerator::global()->bounded(-15, 16);
        const int offsetY = QRandomGenerator::global()->bounded(-15, 16);
        move(originalPos.x() + offsetX, originalPos.y() + offsetY);
      } else {
        move(originalPos);
        wizzAnimating = false;
      }
    });
  }
}

void MainWindow::onWsError(const QString& code, const QString& message) {
  QMessageBox::warning(this, tr("WebSocket"), tr("%1: %2").arg(code, message));
}

void MainWindow::onWsDisconnected(const QString& reason) {
  setChatEnabled(false);
  if (suppressDisconnectPopup) {
    suppressDisconnectPopup = false;
    return;
  }
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

void MainWindow::onWizzRequested() {
  if (currentChannelId <= 0) {
    QMessageBox::warning(this, tr("Wizz"), tr("Select a channel first."));
    return;
  }

  qInfo().noquote() << "[UI][WIZZ] channel_id=" << currentChannelId;
  wsClient->sendWizz(currentChannelId);
  rightPanel->focusInput();
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

void MainWindow::onRegisterRequested() {
  qInfo() << "[UI] REGISTER_REQUESTED";
  loginWidget->setErrorText(QString());
  loginWidget->hide();
  registerWidget->show();
}

void MainWindow::onRegisterConfirmed(const QString& username,
                                     const QString& email,
                                     const QString& password) {
  qInfo() << "[UI] REGISTER_CONFIRMED username=" << username
          << "email=" << email;
  if (authManager) {
    authManager->registerUser(username, email, password);
  }
}

void MainWindow::onRegisterCancelled() {
  qInfo() << "[UI] REGISTER_CANCELLED";
  registerWidget->hide();
  loginWidget->show();
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

QString MainWindow::getUserInitials(const QString& username) const {
  if (username.isEmpty()) {
    return "??";
  }

  // Split username by spaces and take first letter of each word
  QStringList words = username.split(' ', Qt::SkipEmptyParts);
  QString initials;

  if (words.isEmpty()) {
    return "??";
  }

  // Take first letter of first word
  initials += words.first()[0].toUpper();

  // If there's a second word, add its first letter too
  if (words.size() > 1) {
    initials += words[1][0].toUpper();
  }

  return initials;
}

void MainWindow::onNewInvitationReceived(ServerSend::ChannelInvitation& invit) {
  // incomingModel // update incomingModel avec nouveau widget d'invitation item
  this->incomingInvitationModel->addInvitation(invit);

  this->userHomeWidget->setIncomingInvitationModels(incomingInvitationModel);
  return;
}

void MainWindow::applyInvitationAccepted(
    int64_t invitationChannelId, const ServerSend::ChannelInfo& channel) {
  if (channelPanel) {
    channelPanel->addChannel(channel);
  }

  channelTitles.insert(channel.id_channel,
                       QString::fromStdString(channel.title));
  for (const auto& participant : channel.participants) {
    userNamesById.insert(participant.id_user,
                         QString::fromStdString(participant.username));
  }

  if (incomingInvitationModel) {
    incomingInvitationModel->removeInvitation(invitationChannelId);
  }
}

void MainWindow::onNewInvitationAccepted(
    ServerSend::AcceptInvitationResponse& invit) {
  applyInvitationAccepted(invit.channel.id_channel, invit.channel);
  this->userHomeWidget->setIncomingInvitationModels(incomingInvitationModel);
}

void MainWindow::onUserJoinedChannel(
    const ServerSend::UserJoinedNotification& notification) {
  qInfo().noquote() << "[WS][USER_JOINED_HANDLER] channel_id="
                    << notification.id_channel
                    << " user_id=" << notification.contact.id_user;

  userNamesById.insert(notification.contact.id_user,
                       QString::fromStdString(notification.contact.username));

  if (!channelPanel) {
    return;
  }

  const ServerSend::ChannelInfo* existingChannel =
      channelPanel->getChannelInfo(notification.id_channel);
  if (existingChannel) {
    return;
  }

  if (!outgoingInvitationModel) {
    return;
  }

  const ServerSend::ChannelInfo* outgoing =
      outgoingInvitationModel->getInvitationById(notification.id_channel);
  if (!outgoing) {
    return;
  }

  ServerSend::ChannelInfo activatedChannel = *outgoing;
  const bool participantAlreadyPresent =
      std::any_of(activatedChannel.participants.begin(),
                  activatedChannel.participants.end(),
                  [&notification](const ServerSend::Contact& contact) {
                    return contact.id_user == notification.contact.id_user;
                  });

  if (!participantAlreadyPresent) {
    activatedChannel.participants.push_back(notification.contact);
  }
  activatedChannel.is_group = activatedChannel.participants.size() > 2;

  channelPanel->addChannel(activatedChannel);
  outgoingInvitationModel->removeInvitation(notification.id_channel);

  qInfo() << "[UI][USER_JOINED] Activated channel" << notification.id_channel
          << "for current user";
}

void MainWindow::onUserLeftChannel(
    const ServerSend::UserLeftNotification& notification) {
  qInfo().noquote() << "[WS][USER_LEFT_HANDLER] channel_id="
                    << notification.id_channel
                    << " user_id=" << notification.id_user
                    << " current_user=" << currentUserId;

  auto removeChannelFromUi = [this](int64_t channelId) {
    if (currentChannelId == channelId) {
      rightPanel->hide();
      if (userHomeWidget) {
        userHomeWidget->show();
      }
      currentChannelId = -1;
    }
    if (channelPanel) {
      channelPanel->removeChannel(channelId);
    }
  };

  // If the current user left the channel, remove it from the UI
  if (notification.id_user == currentUserId) {
    removeChannelFromUi(notification.id_channel);

    qInfo() << "[UI][USER_LEFT_SELF] Channel" << notification.id_channel
            << "removed from list";
    return;
  }

  if (!channelPanel) {
    return;
  }

  const ServerSend::ChannelInfo* channel =
      channelPanel->getChannelInfo(notification.id_channel);
  if (!channel) {
    return;
  }

  ServerSend::ChannelInfo updatedChannel = *channel;
  if (channelPanel) {
    updatedChannel.unread_count =
        channelPanel->unreadCountForChannel(notification.id_channel);
  }
  updatedChannel.participants.erase(
      std::remove_if(updatedChannel.participants.begin(),
                     updatedChannel.participants.end(),
                     [&notification](const ServerSend::Contact& contact) {
                       return contact.id_user == notification.id_user;
                     }),
      updatedChannel.participants.end());

  const std::size_t remainingParticipants = updatedChannel.participants.size();
  updatedChannel.is_group = remainingParticipants > 2;

  if (remainingParticipants <= 1) {
    removeChannelFromUi(notification.id_channel);
    qInfo() << "[UI][USER_LEFT_REMOVE] Removing channel"
            << notification.id_channel
            << "remaining=" << static_cast<int>(remainingParticipants);
    return;
  }

  if (!channelPanel->updateChannel(updatedChannel)) {
    qWarning() << "[UI][USER_LEFT_UPDATE_FAILED] channel_id="
               << notification.id_channel;
  } else {
    qInfo() << "[UI][USER_LEFT_UPDATE] channel_id=" << notification.id_channel
            << "remaining=" << static_cast<int>(remainingParticipants)
            << "is_group=" << updatedChannel.is_group;
  }
}

void MainWindow::onLogoutRequested() {
  suppressDisconnectPopup = true;
  authManager->logout(authToken);
  currentUserId = -1;
  // TODO finish to purge all data
}

void MainWindow::onLogoutSucceeded() {
  // 1. Disconnect WebSocket
  wsClient->disconnectFromServer();

  // 2. Clear all state
  authToken.clear();
  currentUser.clear();
  currentUserId = -1;
  currentChannelId = -1;
  userNamesById.clear();
  channelTitles.clear();

  // 3. Clear all models / caches shown in UI
  if (incomingInvitationModel) {
    incomingInvitationModel->clear();
  }
  if (outgoingInvitationModel) {
    outgoingInvitationModel->clear();
  }
  if (userHomeWidget) {
    userHomeWidget->setUsernameCache(nullptr);
    userHomeWidget->setIncomingInvitationModels(incomingInvitationModel);
    userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
  }
  if (channelPanel) {
    channelPanel->clearChannels();
    channelPanel->setUserInfo("", "??");
  }
  rightPanel->clearMessages();
  rightPanel->setChatTitle("Select a chat to start messaging");
  setChatEnabled(false);

  // 4. Hide right panel, show login
  if (userHomeWidget) userHomeWidget->hide();
  rightPanel->show();

  // 5. Back to login page
  ui->stackedWidget->setCurrentIndex(0);
  setWindowTitle("WizzMania");
}

void MainWindow::onNewInvitationRejected(
    ServerSend::RejectInvitationResponse& rejection) {
  if (rejection.type == WizzMania::MessageType::CANCEL_INVITATION) {
    if (rejection.contact.id_user == this->currentUserId) {
      if (outgoingInvitationModel) {
        outgoingInvitationModel->removeInvitation(rejection.id_channel);
        userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
      }
    } else {
      if (incomingInvitationModel) {
        incomingInvitationModel->removeInvitation(rejection.id_channel);
        userHomeWidget->setIncomingInvitationModels(incomingInvitationModel);
      }
      statusBar()->showMessage(tr("Invitation canceled by creator."), 2500);
    }
    return;
  }

  if (rejection.contact.id_user == this->currentUserId) {
    this->incomingInvitationModel->removeInvitation(rejection.id_channel);
    this->userHomeWidget->setIncomingInvitationModels(incomingInvitationModel);
  } else {
    // in case the creator receive the signal
    this->outgoingInvitationModel->removeInvitation(rejection.id_channel);
    this->userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
  }
}

void MainWindow::onNewChannelCreated(
    ServerSend::CreateChannelResponse& channel) {
  this->outgoingInvitationModel->addInvitation(channel.channel);
  this->userHomeWidget->setOutgoingInvitationModels(outgoingInvitationModel);
}