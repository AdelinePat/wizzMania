#include "widgets/right_panel_widget.hpp"

RightPanelWidget::RightPanelWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(10);

  QWidget* headerRow = new QWidget(this);
  QHBoxLayout* headerLayout = new QHBoxLayout(headerRow);
  headerLayout->setContentsMargins(0, 0, 0, 0);
  headerLayout->setSpacing(8);

  titleLabel = new QLabel("Select a chat to start messaging", headerRow);
  titleLabel->setObjectName("chatTitleLabel");

  leaveButton = new QPushButton("Leave", headerRow);
  leaveButton->setObjectName("channelLeaveBtn");
  leaveButton->setMinimumHeight(32);

  headerLayout->addWidget(titleLabel, 1);
  headerLayout->addWidget(leaveButton, 0, Qt::AlignRight | Qt::AlignVCenter);

  messagesList = new QListWidget(this);
  messagesList->setObjectName("messagesList");
  messagesList->setWordWrap(true);
  messagesList->setSpacing(8);
  messagesList->setSelectionMode(QAbstractItemView::NoSelection);
  messagesList->setFocusPolicy(Qt::NoFocus);

  QWidget* inputRow = new QWidget(this);
  QHBoxLayout* inputLayout = new QHBoxLayout(inputRow);
  inputLayout->setContentsMargins(0, 0, 0, 0);
  inputLayout->setSpacing(6);

  messageInput = new QLineEdit(inputRow);
  messageInput->setMinimumHeight(40);
  messageInput->setPlaceholderText("Type a message...");
  messageInput->setObjectName("messageInput");

  wizzButton = new QPushButton("⚡", inputRow);
  wizzButton->setMinimumSize(80, 40);
  wizzButton->setObjectName("wizzButton");

  sendButton = new QPushButton("Send", inputRow);
  sendButton->setMinimumSize(80, 40);
  sendButton->setObjectName("sendButton");

  inputLayout->addWidget(messageInput, 1);
  inputLayout->addWidget(wizzButton);
  inputLayout->addWidget(sendButton);

  rootLayout->addWidget(headerRow);
  rootLayout->addWidget(messagesList, 1);
  rootLayout->addWidget(inputRow);

  connect(sendButton, &QPushButton::clicked, this, [this]() {
    const QString text = messageInput->text().trimmed();
    if (text.isEmpty()) {
      return;
    }
    emit sendRequested(text);
    messageInput->clear();
  });

  connect(messageInput, &QLineEdit::returnPressed, this, [this]() {
    const QString text = messageInput->text().trimmed();
    if (text.isEmpty()) {
      return;
    }
    emit sendRequested(text);
    messageInput->clear();
  });

  connect(wizzButton, &QPushButton::clicked, this,
          [this]() { emit wizzRequested(); });

  connect(leaveButton, &QPushButton::clicked, this,
          [this]() { emit leaveChannelRequested(); });
}

void RightPanelWidget::setChatTitle(const QString& title) {
  titleLabel->setText(title);
}

void RightPanelWidget::clearMessages() { messagesList->clear(); }

void RightPanelWidget::addPlainMessage(const QString& text) {
  messagesList->addItem(text);
  messagesList->scrollToBottom();
}

void RightPanelWidget::addMessageWidget(QWidget* widget) {
  if (!widget) {
    return;
  }

  widget->setParent(messagesList);
  widget->adjustSize();

  QListWidgetItem* item = new QListWidgetItem();
  item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
  item->setSizeHint(widget->sizeHint());
  messagesList->addItem(item);
  messagesList->setItemWidget(item, widget);
  messagesList->scrollToBottom();
}

void RightPanelWidget::setInputEnabled(bool enabled) {
  sendButton->setEnabled(enabled);
  wizzButton->setEnabled(enabled);
  messageInput->setEnabled(enabled);
  leaveButton->setEnabled(enabled);
}

void RightPanelWidget::focusInput() { messageInput->setFocus(); }
