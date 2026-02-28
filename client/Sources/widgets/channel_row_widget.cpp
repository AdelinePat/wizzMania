#include "widgets/channel_row_widget.hpp"

ChannelRowWidget::ChannelRowWidget(int64_t channelId, const QString& title,
                                   const QString& preview, int unreadCount,
                                   bool isGroup, QWidget* parent)
    : QWidget(parent), channelId(channelId) {
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

  // Leave button (dangerous action)
  QPushButton* leaveBtn = new QPushButton("Leave", this);
  leaveBtn->setObjectName("channelLeaveBtn");
  connect(leaveBtn, &QPushButton::clicked, this,
          [this, channelId = this->channelId]() {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Leave Channel");
            msgBox.setText("Are you sure you want to leave this channel?");
            msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            msgBox.button(QMessageBox::Yes)->setText("Confirm");

            if (msgBox.exec() == QMessageBox::Yes) {
              emit leaveChannelRequested(channelId);
            }
          });
  rootLayout->addWidget(leaveBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
}
