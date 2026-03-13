#include "widgets/message_item_widget.hpp"

MessageItemWidget::MessageItemWidget(const ServerSend::Message& message,
                                     int64_t currentUserId,
                                     const QString& senderName,
                                     const QString& resolvedBody,
                                     QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* rowLayout = new QHBoxLayout(this);
  rowLayout->setContentsMargins(8, 2, 8, 2);
  rowLayout->setSpacing(0);

  // ── System messages ───────────────────────────────────────────
  if (message.is_system) {
    const QString displayText = resolvedBody.isEmpty()
                                    ? QString::fromStdString(message.body)
                                    : resolvedBody;
    QWidget* sysWrap = new QWidget(this);
    sysWrap->setObjectName("systemMsgWrap");
    QHBoxLayout* sysLayout = new QHBoxLayout(sysWrap);
    sysLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* systemLabel = new QLabel(displayText, sysWrap);
    systemLabel->setWordWrap(true);
    systemLabel->setObjectName("systemLabel");
    systemLabel->setAlignment(Qt::AlignCenter);
    sysLayout->addStretch();
    sysLayout->addWidget(systemLabel);
    sysLayout->addStretch();
    rowLayout->addWidget(sysWrap);
    return;
  }

  const bool isCurrentUser = (message.id_sender == currentUserId);
  const QString timestamp = QString::fromStdString(message.timestamp);
  const QString body      = QString::fromStdString(message.body);

  // ── Avatar ────────────────────────────────────────────────────
  // Derive 1–2 initials from senderName
  QString initials;
  const QStringList parts = senderName.split(' ', Qt::SkipEmptyParts);
  if (!parts.isEmpty()) {
    initials = parts[0].left(1).toUpper();
    if (parts.size() > 1) initials += parts[1].left(1).toUpper();
  }
  if (initials.isEmpty()) initials = "?";

  QLabel* avatarLabel = new QLabel(initials, this);
  avatarLabel->setObjectName(isCurrentUser ? "avatarSelf" : "avatarOther");
  avatarLabel->setFixedSize(34, 34);
  avatarLabel->setAlignment(Qt::AlignCenter);

  // ── Message column ────────────────────────────────────────────
  QWidget* messageColumn = new QWidget(this);
  messageColumn->setObjectName("transparentWidget");
  messageColumn->setMaximumWidth(500);
  QVBoxLayout* columnLayout = new QVBoxLayout(messageColumn);
  columnLayout->setContentsMargins(0, 0, 0, 0);
  columnLayout->setSpacing(3);

  // Sender name (hidden for own messages)
  if (!isCurrentUser) {
    QLabel* senderLabel = new QLabel(senderName, messageColumn);
    senderLabel->setObjectName("senderLabel");
    columnLayout->addWidget(senderLabel);
  }

  // Bubble
  QFrame* bubbleFrame = new QFrame(messageColumn);
  bubbleFrame->setObjectName("bubbleFrame");
  bubbleFrame->setProperty("owner", isCurrentUser ? "self" : "other");

  QVBoxLayout* bubbleLayout = new QVBoxLayout(bubbleFrame);
  bubbleLayout->setContentsMargins(12, 8, 12, 8);
  bubbleLayout->setSpacing(0);

  QLabel* bodyLabel = new QLabel(body, bubbleFrame);
  bodyLabel->setWordWrap(true);
  bodyLabel->setObjectName("bodyLabel");
  bodyLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  bubbleLayout->addWidget(bodyLabel);

  columnLayout->addWidget(bubbleFrame);

  // Timestamp
  QLabel* timeLabel = new QLabel(DisplayUtils::formatTimestamp(timestamp), messageColumn);
  timeLabel->setObjectName("timeLabel");
  timeLabel->setAlignment(isCurrentUser ? Qt::AlignLeft : Qt::AlignRight);
  columnLayout->addWidget(timeLabel);

  // ── Row assembly (own = left+stretch, other = stretch+right) ──
  // Per existing convention: isCurrentUser → left side, other → right side
  if (isCurrentUser) {
    rowLayout->addWidget(avatarLabel, 0, Qt::AlignTop);
    rowLayout->addSpacing(8);
    rowLayout->addWidget(messageColumn, 0, Qt::AlignLeft | Qt::AlignTop);
    rowLayout->addStretch();
  } else {
    rowLayout->addStretch();
    rowLayout->addWidget(messageColumn, 0, Qt::AlignRight | Qt::AlignTop);
    rowLayout->addSpacing(8);
    rowLayout->addWidget(avatarLabel, 0, Qt::AlignTop);
  }
}
