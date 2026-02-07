#include "mainwindow.h"

#include <QDateTime>
#include <QVBoxLayout>

#include "loginwidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), loginWidget(nullptr) {
  ui->setupUi(this);

  // Create and add login widget to the login page
  loginWidget = new LoginWidget(this);
  QVBoxLayout* loginLayout = new QVBoxLayout(ui->loginPage);
  loginLayout->setContentsMargins(0, 0, 0, 0);
  loginLayout->addWidget(loginWidget);

  // Connect login signal
  connect(loginWidget, &LoginWidget::loginSuccessful, this,
          &MainWindow::onLoginSuccessful);

  // Setup chat view connections
  setupChatView();

  // Start on login page
  ui->stackedWidget->setCurrentIndex(0);
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

  // Switch to chat view
  ui->stackedWidget->setCurrentIndex(1);

  // Add some demo chat groups
  addDemoChats();

  setWindowTitle(QString("WizzMania - %1").arg(username));
}

void MainWindow::addDemoChats() {
  // Add some demo chat groups for testing
  ui->chatGroupsList->clear();
  ui->chatGroupsList->addItem("General Chat");
  ui->chatGroupsList->addItem("Team Alpha");
  ui->chatGroupsList->addItem("Random");
  ui->chatGroupsList->addItem("Tech Talk");
  ui->chatGroupsList->addItem("Off Topic");
}

void MainWindow::onChatSelected(int row) {
  if (row < 0) return;

  QString chatName = ui->chatGroupsList->item(row)->text();
  ui->chatTitleLabel->setText(chatName);

  // Clear messages and show some demo messages
  ui->messagesList->clear();

  // Add some demo messages for testing
  ui->messagesList->addItem(QString("[System] Welcome to %1").arg(chatName));
  ui->messagesList->addItem("[Alice] Hey everyone!");
  ui->messagesList->addItem("[Bob] Hello!");
  ui->messagesList->addItem(
      QString("[%1] Just joined the chat").arg(currentUser));
}

void MainWindow::onSendMessage() {
  QString message = ui->messageInput->text().trimmed();
  if (message.isEmpty()) return;

  // Format and add message
  QString timestamp = QDateTime::currentDateTime().toString("hh:mm");
  QString formattedMessage =
      QString("[%1] %2: %3").arg(timestamp, currentUser, message);

  ui->messagesList->addItem(formattedMessage);
  ui->messagesList->scrollToBottom();

  // Clear input
  ui->messageInput->clear();
  ui->messageInput->setFocus();
}
