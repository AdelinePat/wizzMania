#include "widgets/channel_panel_widget.hpp"

ChannelPanelWidget::ChannelPanelWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(8);

  QLabel* titleLabel = new QLabel("WizzMania", this);
  titleLabel->setObjectName("appLabel");

  // User portrait + name (clickable)
  userPortraitBtn = new QPushButton("??", this);
  userPortraitBtn->setFixedSize(48, 48);
  userPortraitBtn->setObjectName("userPortraitBtn");
  userPortraitName = new QLabel("", this);
  userPortraitName->setObjectName("userPortraitName");

  QWidget* topWidget = new QWidget(this);
  QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
  topLayout->setContentsMargins(0, 0, 0, 0);
  topLayout->setSpacing(6);
  topLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
  topLayout->addWidget(userPortraitBtn, 0, Qt::AlignHCenter);
  topLayout->addWidget(userPortraitName, 0, Qt::AlignHCenter);

  channelsList = new QListWidget(this);
  channelsList->setObjectName("channelsList");

  rootLayout->addWidget(topWidget);
  rootLayout->addWidget(channelsList, 1);

  connect(userPortraitBtn, &QPushButton::clicked, this,
          [this]() { emit userHomeRequested(); });

  connect(channelsList, &QListWidget::currentItemChanged, this,
          [this](QListWidgetItem* current, QListWidgetItem*) {
            if (!current) {
              return;
            }
            const int64_t channelId = current->data(Qt::UserRole).toLongLong();
            if (channelId <= 0) {
              return;
            }
            emit channelSelected(channelId,
                                 current->data(Qt::UserRole + 1).toString());
          });

  // Ensure clicking an already-selected channel still triggers selection
  connect(channelsList, &QListWidget::itemClicked, this,
          [this](QListWidgetItem* item) {
            if (!item) return;
            const int64_t channelId = item->data(Qt::UserRole).toLongLong();
            if (channelId <= 0) return;
            emit channelSelected(channelId,
                                 item->data(Qt::UserRole + 1).toString());
          });
}

void ChannelPanelWidget::setChannels(
    const std::vector<ServerSend::ChannelInfo>& channels) {
  channelsList->clear();

  if (channels.empty()) {
    QListWidgetItem* item = new QListWidgetItem("No channels", channelsList);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setData(Qt::UserRole, static_cast<qint64>(-1));
    return;
  }

  for (const auto& channel : channels) {
    const QString title = QString::fromStdString(channel.title);
    const QString preview = QString::fromStdString(channel.last_message.body);

    QListWidgetItem* item = new QListWidgetItem();
    item->setData(Qt::UserRole, static_cast<qint64>(channel.id_channel));
    item->setData(Qt::UserRole + 1, title);
    // store preview, unread_count and is_group in item data for later updates
    item->setData(Qt::UserRole + 2, preview);
    item->setData(Qt::UserRole + 3, static_cast<qint64>(channel.unread_count));
    item->setData(Qt::UserRole + 4, channel.is_group);

    ChannelRowWidget* row = new ChannelRowWidget(
        title, preview, static_cast<int>(channel.unread_count),
        channel.is_group, channelsList);
    item->setSizeHint(row->sizeHint());
    channelsList->addItem(item);
    channelsList->setItemWidget(item, row);
  }
}

void ChannelPanelWidget::updateChannelOnNewMessage(int64_t channelId,
                                                   const QString& preview,
                                                   bool incrementUnread) {
  for (int i = 0; i < channelsList->count(); ++i) {
    QListWidgetItem* item = channelsList->item(i);
    if (!item) continue;
    qint64 id = item->data(Qt::UserRole).toLongLong();
    if (id != channelId) continue;

    QString title = item->data(Qt::UserRole + 1).toString();
    qint64 unread = item->data(Qt::UserRole + 3).toLongLong();
    if (incrementUnread) unread += 1;

    // Update stored data
    item->setData(Qt::UserRole + 2, preview);
    item->setData(Qt::UserRole + 3, unread);

    // Replace widget with updated preview/unread
    bool isGroup = item->data(Qt::UserRole + 4).toBool();
    ChannelRowWidget* newRow = new ChannelRowWidget(
        title, preview, static_cast<int>(unread), isGroup, channelsList);
    // preserve is_group if previously set: try to retrieve from existing
    QWidget* existing = channelsList->itemWidget(item);
    if (existing) {
      // Heuristic: extract isGroup from the icon label text present in the
      // existing widget's children (best-effort). If not found, keep false.
    }

    item->setSizeHint(newRow->sizeHint());
    channelsList->setItemWidget(item, newRow);
    return;
  }
}

void ChannelPanelWidget::setUserInfo(const QString& username,
                                     const QString& initials) {
  if (userPortraitBtn) {
    userPortraitBtn->setText(initials);
  }
  if (userPortraitName) {
    userPortraitName->setText(username);
  }
}
