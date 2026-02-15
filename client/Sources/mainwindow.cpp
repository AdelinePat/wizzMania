#include "mainwindow.hpp"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      loginWidget(nullptr),
      wsClient(new WebSocketClient(this)) {
  ui->setupUi(this);

  // Create and add login widget to the login page
  loginWidget = new LoginWidget(this);
  QVBoxLayout* loginLayout = new QVBoxLayout(ui->loginPage);
  loginLayout->setContentsMargins(0, 0, 0, 0);
  loginLayout->addWidget(loginWidget);

  // Connect login signal
  connect(loginWidget, &LoginWidget::loginSuccessful, this,
          &MainWindow::onLoginSuccessful);

  // WebSocket signals
  connect(wsClient, &WebSocketClient::authenticated, this,
          &MainWindow::onWsAuthenticated);
  connect(wsClient, &WebSocketClient::initialDataReceived, this,
          &MainWindow::onInitialDataReceived);
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
  // Connect chat list selection
  connect(ui->chatGroupsList, &QListWidget::currentRowChanged, this,
          &MainWindow::onChatSelected);

  // Connect send button and enter key
  connect(ui->sendButton, &QPushButton::clicked, this,
          &MainWindow::onSendMessage);
  connect(ui->messageInput, &QLineEdit::returnPressed, this,
          &MainWindow::onSendMessage);

  // Set initial splitter sizes (250px for left panel, rest for right)
  ui->chatSplitter->setSizes({250, 650});
}

void MainWindow::onLoginSuccessful(const QString& username,
                                   const QString& token) {
  currentUser = username;
  authToken = token;

  ui->chatTitleLabel->setText("Connecting to server...");
  setChatEnabled(false);

  wsClient->connectWithToken(token);

  // Switch to chat view
  ui->stackedWidget->setCurrentIndex(1);

  setWindowTitle(QString("WizzMania - %1").arg(username));
}

void MainWindow::onWsAuthenticated(int64_t idUser) {
  currentUserId = idUser;
  setChatEnabled(true);
  ui->chatTitleLabel->setText("Select a chat to start messaging");
}

void MainWindow::onInitialDataReceived(
    const ServerSend::InitialDataResponse& data) {
  populateChannels(data.channels);

  if (data.channels.empty()) {
    ui->messagesList->clear();
    ui->messagesList->addItem("No channels available yet.");
  }
}

void MainWindow::onNewMessageReceived(
    const ServerSend::NewMessageBroadcast& msg) {
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

void MainWindow::addDemoChats() {
  // Add some demo chat groups for testing
  ui->chatGroupsList->clear();
  const QStringList demoChats = {"General Chat", "Team Alpha", "Random",
                                 "Tech Talk", "Off Topic"};
  for (int i = 0; i < demoChats.size(); ++i) {
    QListWidgetItem* item = new QListWidgetItem(demoChats[i]);
    item->setData(Qt::UserRole, static_cast<qint64>(i + 1));
    ui->chatGroupsList->addItem(item);
  }
}

void MainWindow::populateChannels(
    const std::vector<ServerSend::ChannelInfo>& channels) {
  ui->chatGroupsList->clear();
  channelTitles.clear();

  if (channels.empty()) {
    QListWidgetItem* item = new QListWidgetItem("No channels");
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setData(Qt::UserRole, static_cast<qint64>(-1));
    ui->chatGroupsList->addItem(item);
    return;
  }

  for (const auto& channel : channels) {
    const QString title = QString::fromStdString(channel.title);
    QListWidgetItem* item = new QListWidgetItem(title);
    item->setData(Qt::UserRole, static_cast<qint64>(channel.id_channel));
    ui->chatGroupsList->addItem(item);
    channelTitles.insert(channel.id_channel, title);
  }
}

void MainWindow::onChatSelected(int row) {
  if (row < 0) return;

  QListWidgetItem* item = ui->chatGroupsList->item(row);
  const qint64 channelId = item->data(Qt::UserRole).toLongLong();
  currentChannelId = channelId;

  QString chatName = item->text();
  ui->chatTitleLabel->setText(chatName);

  ui->messagesList->clear();

  if (channelId <= 0) {
    return;
  }

  ui->messagesList->addItem("Loading messages...");
  wsClient->openChannel(channelId);
}

void MainWindow::onSendMessage() {
  QString message = ui->messageInput->text().trimmed();
  if (message.isEmpty()) return;

  if (currentChannelId <= 0) {
    QMessageBox::warning(this, tr("Message"), tr("Select a channel first."));
    return;
  }

  wsClient->sendMessage(currentChannelId, message);

  ui->messageInput->clear();
  ui->messageInput->setFocus();
}

void MainWindow::appendMessageToView(int64_t channelId,
                                     const ServerSend::Message& msg) {
  Q_UNUSED(channelId);

  const QString sender =
      msg.is_system ? QString("System") : QString::number(msg.id_sender);
  const QString timestamp = QString::fromStdString(msg.timestamp);
  const QString body = QString::fromStdString(msg.body);

  const QString line = QString("[%1] %2: %3").arg(timestamp, sender, body);
  ui->messagesList->addItem(line);
  ui->messagesList->scrollToBottom();
}

void MainWindow::setChatEnabled(bool enabled) {
  ui->sendButton->setEnabled(enabled);
  ui->messageInput->setEnabled(enabled);
}
