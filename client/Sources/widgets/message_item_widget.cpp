#include "widgets/message_item_widget.hpp"

MessageItemWidget::MessageItemWidget(const ServerSend::Message& message,
                                     int64_t currentUserId,
                                     const QString& senderName,
                                     const QString& resolvedBody,
                                     QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* rowLayout = new QHBoxLayout(this);
  rowLayout->setContentsMargins(4, 4, 4, 4);

  if (message.is_system) {
    const QString displayText = resolvedBody.isEmpty()
                                    ? QString::fromStdString(message.body)
                                    : resolvedBody;
    QLabel* systemLabel = new QLabel(displayText, this);
    systemLabel->setWordWrap(true);
    systemLabel->setObjectName("systemLabel");
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
  senderLabel->setObjectName("senderLabel");
  // senderLabel->setProperty("owner", isCurrentUser ? "self" : "other");

  QFrame* bubbleFrame = new QFrame(messageColumn);
  bubbleFrame->setObjectName("bubbleFrame");
  bubbleFrame->setProperty("owner", isCurrentUser ? "self" : "other");

  QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleFrame);
  bubbleLayout->setContentsMargins(8, 6, 8, 6);
  bubbleLayout->setSpacing(6);

  QLabel* bodyLabel = new QLabel(body, bubbleFrame);
  bodyLabel->setWordWrap(true);
  bodyLabel->setObjectName("bodyLabel");
  bubbleLayout->addWidget(bodyLabel);

  QLabel* timeLabel = new QLabel(DisplayUtils::formatTimestamp(timestamp), messageColumn);
  timeLabel->setObjectName("timeLabel");
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
