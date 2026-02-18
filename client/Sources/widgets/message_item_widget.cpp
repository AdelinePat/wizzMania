#include "widgets/message_item_widget.hpp"

MessageItemWidget::MessageItemWidget(const ServerSend::Message& message,
                                     int64_t currentUserId,
                                     const QString& senderName, QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* rowLayout = new QHBoxLayout(this);
  rowLayout->setContentsMargins(8, 6, 8, 6);

  if (message.is_system) {
    QLabel* systemLabel =
        new QLabel(QString::fromStdString(message.body), this);
    systemLabel->setWordWrap(true);
    systemLabel->setStyleSheet(
        "QLabel { color: #9aa6b2; font-style: italic; font-size: 12px; "
        "padding: 2px 8px; }");
    rowLayout->addWidget(systemLabel, 0, Qt::AlignCenter);
    return;
  }

  const bool isCurrentUser = (message.id_sender == currentUserId);
  const QString timestamp = QString::fromStdString(message.timestamp);
  const QString body = QString::fromStdString(message.body);

  QWidget* messageColumn = new QWidget(this);
  messageColumn->setMaximumWidth(520);
  QVBoxLayout* columnLayout = new QVBoxLayout(messageColumn);
  columnLayout->setContentsMargins(0, 0, 0, 0);
  columnLayout->setSpacing(4);

  QLabel* senderLabel = new QLabel(senderName, messageColumn);
  senderLabel->setStyleSheet(isCurrentUser
                                 ? "QLabel { color: #d9ffe8; font-size: 11px; "
                                   "font-weight: 600; padding-left: 2px; }"
                                 : "QLabel { color: #91b9d7; font-size: 11px; "
                                   "font-weight: 600; padding-left: 2px; }");

  QFrame* bubbleFrame = new QFrame(messageColumn);
  bubbleFrame->setStyleSheet(
      isCurrentUser
          ? "QFrame { background-color: #1f8b4c; border-radius: 12px; }"
          : "QFrame { background-color: #12212e; border-radius: 12px; }");

  QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleFrame);
  bubbleLayout->setContentsMargins(12, 10, 12, 10);
  bubbleLayout->setSpacing(6);

  QLabel* bodyLabel = new QLabel(body, bubbleFrame);
  bodyLabel->setWordWrap(true);
  bodyLabel->setStyleSheet("QLabel { color: #e4edf5; font-size: 13px; }");
  bubbleLayout->addWidget(bodyLabel);

  QLabel* timeLabel = new QLabel(timestamp, messageColumn);
  timeLabel->setStyleSheet("QLabel { color: #9aa6b2; font-size: 10px; }");
  timeLabel->setAlignment(isCurrentUser ? Qt::AlignLeft : Qt::AlignRight);

  columnLayout->addWidget(senderLabel);
  columnLayout->addWidget(bubbleFrame);
  columnLayout->addWidget(timeLabel);

  if (isCurrentUser) {
    rowLayout->addWidget(messageColumn, 0, Qt::AlignLeft);
    rowLayout->addStretch();
  } else {
    rowLayout->addStretch();
    rowLayout->addWidget(messageColumn, 0, Qt::AlignRight);
  }
}
