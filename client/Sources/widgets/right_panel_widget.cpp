#include "widgets/right_panel_widget.hpp"

RightPanelWidget::RightPanelWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(10);

  titleLabel = new QLabel("Select a chat to start messaging", this);
  titleLabel->setStyleSheet(
      "QLabel { color: rgb(200, 200, 200); padding: 5px; "
      "background-color: rgb(0, 22, 33); border-radius: 4px; "
      "font-size: 12px; font-weight: 700; }");

  messagesList = new QListWidget(this);
  messagesList->setStyleSheet(
      "QListWidget {"
      "  background-color: rgb(0, 22, 33);"
      "  color: rgb(200, 200, 200);"
      "  border: none;"
      "  border-radius: 4px;"
      "}");
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
  messageInput->setStyleSheet(
      "QLineEdit {"
      "  background-color: rgb(0, 27, 41);"
      "  color: rgb(200, 200, 200);"
      "  border: 1px solid rgb(66, 126, 157);"
      "  border-radius: 4px;"
      "  padding: 8px;"
      "}");

  sendButton = new QPushButton("Send", inputRow);
  sendButton->setMinimumSize(80, 40);
  sendButton->setStyleSheet(
      "QPushButton {"
      "  background-color: rgb(82, 134, 77);"
      "  color: white;"
      "  border: none;"
      "  border-radius: 4px;"
      "  font-weight: bold;"
      "}"
      "QPushButton:hover {"
      "  background-color: rgb(92, 144, 87);"
      "}"
      "QPushButton:pressed {"
      "  background-color: rgb(72, 124, 67);"
      "}");

  inputLayout->addWidget(messageInput, 1);
  inputLayout->addWidget(sendButton);

  rootLayout->addWidget(titleLabel);
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
  messageInput->setEnabled(enabled);
}

void RightPanelWidget::focusInput() { messageInput->setFocus(); }
