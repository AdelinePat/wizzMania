#include "widgets/channel_row_widget.hpp"

ChannelRowWidget::ChannelRowWidget(const QString& title, const QString& preview,
                                   int unreadCount, bool isGroup,
                                   QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* rootLayout = new QHBoxLayout(this);
  rootLayout->setContentsMargins(4, 4, 4, 4);
  rootLayout->setSpacing(6);

  QLabel* iconLabel = new QLabel(isGroup ? "⊞" : "◈", this);
  iconLabel->setAlignment(Qt::AlignCenter);
  iconLabel->setFixedSize(26, 26);
  iconLabel->setObjectName("channelIconLabel");

  QWidget* textBlock = new QWidget(this);
  QVBoxLayout* textLayout = new QVBoxLayout(textBlock);
  textLayout->setContentsMargins(0, 0, 0, 0);
  textLayout->setSpacing(2);

  QLabel* titleLabel = new QLabel(title, textBlock);
  titleLabel->setObjectName("channelTitleLabel");

  QLabel* previewLabel = new QLabel(preview, textBlock);
  previewLabel->setObjectName("channelPreviewLabel");
  previewLabel->setWordWrap(false);

  textLayout->addWidget(titleLabel);
  textLayout->addWidget(previewLabel);

  rootLayout->addWidget(iconLabel);
  rootLayout->addWidget(textBlock, 1);

  if (unreadCount > 0) {
    QLabel* unreadLabel = new QLabel(QString::number(unreadCount), this);
    unreadLabel->setAlignment(Qt::AlignCenter);
    unreadLabel->setMinimumWidth(18);
    unreadLabel->setObjectName("channelUnreadLabel");
    rootLayout->addWidget(unreadLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
  }
}
