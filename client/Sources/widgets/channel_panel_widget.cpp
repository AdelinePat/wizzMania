#include "widgets/channel_panel_widget.hpp"

void ChannelPanelWidget::applyChannelDataToItem(
    QListWidgetItem* item, const ServerSend::ChannelInfo& channel) {
  if (!item) {
    return;
  }

  item->setData(ChannelModel::ChannelRole::IdChannelRole,
                static_cast<qint64>(channel.id_channel));
  item->setData(ChannelModel::ChannelRole::TitleRole,
                QString::fromStdString(channel.title));
  item->setData(ChannelModel::ChannelRole::LastMessageBodyRole,
                QString::fromStdString(channel.last_message.body));
  item->setData(ChannelModel::ChannelRole::UnreadCountRole,
                static_cast<qint64>(channel.unread_count));
  item->setData(ChannelModel::ChannelRole::IsGroupRole, channel.is_group);
  item->setData(ChannelModel::ChannelRole::LastReadMessageIdRole,
                static_cast<qint64>(channel.last_read_id_message));
}

void ChannelPanelWidget::rebuildChannelRowWidget(QListWidgetItem* item) {
  if (!item || !channelsList) {
    return;
  }

  const int64_t channelId =
      item->data(ChannelModel::ChannelRole::IdChannelRole).toLongLong();
  const QString title =
      item->data(ChannelModel::ChannelRole::TitleRole).toString();
  const QString preview =
      item->data(ChannelModel::ChannelRole::LastMessageBodyRole).toString();
  const int unreadCount = static_cast<int>(
      item->data(ChannelModel::ChannelRole::UnreadCountRole).toLongLong());
  const bool isGroup =
      item->data(ChannelModel::ChannelRole::IsGroupRole).toBool();

  ChannelRowWidget* row = new ChannelRowWidget(
      channelId, title, preview, unreadCount, isGroup, channelsList);
  item->setSizeHint(row->sizeHint());
  channelsList->setItemWidget(item, row);
}

ChannelPanelWidget::ChannelPanelWidget(QWidget* parent) : QWidget(parent) {
  channelModel = new ChannelModel(this);
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

  // Channels header row ("Channels" label + "+" button)
  QWidget* channelsHeader = new QWidget(this);
  QHBoxLayout* headerLayout = new QHBoxLayout(channelsHeader);
  headerLayout->setContentsMargins(8, 4, 8, 4);
  headerLayout->setSpacing(8);

  QLabel* channelsLabel = new QLabel("Channels", this);
  channelsLabel->setObjectName("channelsHeaderLabel");

  createChannelBtn = new QPushButton("+", this);
  createChannelBtn->setObjectName("createChannelBtn");
  createChannelBtn->setFixedSize(24, 24);

  headerLayout->addWidget(channelsLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(createChannelBtn);

  channelsList = new QListWidget(this);
  channelsList->setObjectName("channelsList");

  // Logout button
  logoutBtn = new QPushButton("Logout", this);
  logoutBtn->setObjectName("logoutBtn");

  rootLayout->addWidget(topWidget);
  rootLayout->addWidget(channelsHeader);
  rootLayout->addWidget(channelsList, 1);
  rootLayout->addWidget(logoutBtn);

  connect(userPortraitBtn, &QPushButton::clicked, this,
          [this]() { emit userHomeRequested(); });

  connect(createChannelBtn, &QPushButton::clicked, this,
          [this]() { emit createChannelRequested(); });

  connect(logoutBtn, &QPushButton::clicked, this,
          [this]() { emit logoutRequested(); });

  connect(channelsList, &QListWidget::currentItemChanged, this,
          [this](QListWidgetItem* current, QListWidgetItem*) {
            if (!current) {
              return;
            }
            const int64_t channelId =
                current->data(ChannelModel::ChannelRole::IdChannelRole)
                    .toLongLong();
            if (channelId <= 0) {
              return;
            }
            emit channelSelected(
                channelId,
                current->data(ChannelModel::ChannelRole::TitleRole).toString());
          });

  // Ensure clicking an already-selected channel still triggers selection
  connect(
      channelsList, &QListWidget::itemClicked, this,
      [this](QListWidgetItem* item) {
        if (!item) return;
        const int64_t channelId =
            item->data(ChannelModel::ChannelRole::IdChannelRole).toLongLong();
        if (channelId <= 0) return;
        emit channelSelected(
            channelId,
            item->data(ChannelModel::ChannelRole::TitleRole).toString());
      });
}

void ChannelPanelWidget::setChannels(
    const std::vector<ServerSend::ChannelInfo>& channels) {
  channelsList->clear();

  if (channels.empty()) {
    QListWidgetItem* item = new QListWidgetItem("No channels", channelsList);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setData(ChannelModel::ChannelRole::IdChannelRole,
                  static_cast<qint64>(-1));
    return;
  }

  // for (const auto& channel : channels) {
  //   const QString title = QString::fromStdString(channel.title);
  //   const QString preview =
  //   QString::fromStdString(channel.last_message.body);

  //   QListWidgetItem* item = new QListWidgetItem();
  //   item->setData(ChannelModel::ChannelRole::IdChannelRole,
  //   static_cast<qint64>(channel.id_channel));
  //   item->setData(ChannelModel::ChannelRole::TitleRole, title);
  //   // store preview, unread_count and is_group in item data for later
  //   updates item->setData(ChannelModel::ChannelRole::LastMessageBodyRole,
  //   preview); item->setData(ChannelModel::ChannelRole::UnreadCountRole,
  //   static_cast<qint64>(channel.unread_count));
  //   item->setData(ChannelModel::ChannelRole::IsGroupRole, channel.is_group);

  //   ChannelRowWidget* row = new ChannelRowWidget(
  //       title, preview, static_cast<int>(channel.unread_count),
  //       channel.is_group, channelsList);
  //   item->setSizeHint(row->sizeHint());
  //   channelsList->addItem(item);
  //   channelsList->setItemWidget(item, row);
  // }
  channelModel->setChannels(channels);  // ← model is now the source of truth
  itemByChannelId.clear();              // ← reset the lookup map

  for (const auto& channel : channels) {
    QListWidgetItem* item = new QListWidgetItem();
    applyChannelDataToItem(item, channel);
    channelsList->addItem(item);
    rebuildChannelRowWidget(item);

    itemByChannelId[channel.id_channel] = item;  // ← register in lookup map
  }
}

void ChannelPanelWidget::addChannel(const ServerSend::ChannelInfo& channel) {
  // Check if channel already exists in the display
  if (itemByChannelId.contains(channel.id_channel)) {
    qWarning() << "[UI][CHANNEL_ADD_DUP] id=" << channel.id_channel
               << " already exists, skipping duplicate";
    return;
  }

  ServerSend::ChannelInfo mergedChannel = channel;
  const auto pendingUnreadIt =
      pendingUnreadByChannelId.constFind(channel.id_channel);
  if (pendingUnreadIt != pendingUnreadByChannelId.constEnd()) {
    mergedChannel.unread_count += pendingUnreadIt.value();
  }
  const auto pendingPreviewIt =
      pendingPreviewByChannelId.constFind(channel.id_channel);
  if (pendingPreviewIt != pendingPreviewByChannelId.constEnd()) {
    mergedChannel.last_message.body = pendingPreviewIt.value().toStdString();
  }

  // Add to internal model
  channelModel->addChannel(mergedChannel);

  QListWidgetItem* item = new QListWidgetItem();
  applyChannelDataToItem(item, mergedChannel);
  channelsList->addItem(item);
  rebuildChannelRowWidget(item);

  itemByChannelId[mergedChannel.id_channel] = item;
  pendingUnreadByChannelId.remove(mergedChannel.id_channel);
  pendingPreviewByChannelId.remove(mergedChannel.id_channel);

  qInfo() << "[UI][CHANNEL_ADDED] id=" << mergedChannel.id_channel
          << " title=" << QString::fromStdString(mergedChannel.title);
}

bool ChannelPanelWidget::updateChannel(const ServerSend::ChannelInfo& channel) {
  auto it = itemByChannelId.find(channel.id_channel);
  if (it == itemByChannelId.end()) {
    return false;
  }

  if (!channelModel->updateChannelInfo(channel)) {
    return false;
  }

  QListWidgetItem* item = it.value();
  ServerSend::ChannelInfo mergedChannel = channel;
  mergedChannel.unread_count =
      item->data(ChannelModel::ChannelRole::UnreadCountRole).toLongLong();
  mergedChannel.last_read_id_message =
      item->data(ChannelModel::ChannelRole::LastReadMessageIdRole).toLongLong();

  const auto pendingUnreadIt =
      pendingUnreadByChannelId.constFind(channel.id_channel);
  if (pendingUnreadIt != pendingUnreadByChannelId.constEnd()) {
    mergedChannel.unread_count += pendingUnreadIt.value();
  }
  const auto pendingPreviewIt =
      pendingPreviewByChannelId.constFind(channel.id_channel);
  if (pendingPreviewIt != pendingPreviewByChannelId.constEnd()) {
    mergedChannel.last_message.body = pendingPreviewIt.value().toStdString();
  }

  if (!channelModel->updateChannelInfo(mergedChannel)) {
    return false;
  }

  applyChannelDataToItem(item, mergedChannel);
  rebuildChannelRowWidget(item);
  pendingUnreadByChannelId.remove(channel.id_channel);
  pendingPreviewByChannelId.remove(channel.id_channel);

  return true;
}

int ChannelPanelWidget::unreadCountForChannel(int64_t channelId) const {
  QListWidgetItem* item = itemByChannelId.value(channelId, nullptr);
  if (!item) {
    return 0;
  }
  return static_cast<int>(
      item->data(ChannelModel::ChannelRole::UnreadCountRole).toLongLong());
}

void ChannelPanelWidget::removeChannel(int64_t channelId) {
  auto it = itemByChannelId.find(channelId);
  if (it == itemByChannelId.end()) {
    qWarning() << "[UI][CHANNEL_REMOVE] id=" << channelId
               << "not found in panel";
    return;
  }

  // Remove from model
  channelModel->removeChannel(channelId);

  QListWidgetItem* item = it.value();
  itemByChannelId.erase(it);
  delete channelsList->takeItem(channelsList->row(item));

  qInfo() << "[UI][CHANNEL_REMOVED] id=" << channelId;
}

const ServerSend::ChannelInfo* ChannelPanelWidget::getChannelInfo(
    int64_t channelId) const {
  if (!channelModel) {
    return nullptr;
  }
  return channelModel->getChannelById(channelId);
}

void ChannelPanelWidget::updateChannelOnNewMessage(int64_t channelId,
                                                   const QString& preview,
                                                   bool incrementUnread) {
  QListWidgetItem* item =
      itemByChannelId.value(channelId, nullptr);  // ← one lookup, no loop
  if (!item) {
    pendingPreviewByChannelId[channelId] = preview;
    if (incrementUnread) {
      pendingUnreadByChannelId[channelId] =
          pendingUnreadByChannelId.value(channelId, 0) + 1;
    }
    return;
  }

  QString title = item->data(ChannelModel::ChannelRole::TitleRole).toString();
  qint64 unread =
      item->data(ChannelModel::ChannelRole::UnreadCountRole).toLongLong();
  const qint64 lastReadMessageId =
      item->data(ChannelModel::ChannelRole::LastReadMessageIdRole).toLongLong();
  if (incrementUnread) unread += 1;

  channelModel->updateLastMessage(channelId, preview);
  channelModel->updateChannelUnreadCount(channelId, unread, lastReadMessageId);

  item->setData(ChannelModel::ChannelRole::LastMessageBodyRole, preview);
  item->setData(ChannelModel::ChannelRole::UnreadCountRole, unread);
  rebuildChannelRowWidget(item);
}
void ChannelPanelWidget::updateChannelUnreadCount(int64_t channelId,
                                                  int unreadCount,
                                                  int64_t last_id_message) {
  QListWidgetItem* item =
      itemByChannelId.value(channelId, nullptr);  // ← one lookup, no loop
  if (!item) {
    pendingUnreadByChannelId[channelId] = unreadCount;
    return;
  }

  channelModel->updateChannelUnreadCount(channelId, unreadCount,
                                         last_id_message);

  item->setData(ChannelModel::ChannelRole::UnreadCountRole,
                static_cast<qint64>(unreadCount));
  if (last_id_message != 0) {
    item->setData(ChannelModel::ChannelRole::LastReadMessageIdRole,
                  static_cast<qint64>(last_id_message));
  }
  rebuildChannelRowWidget(item);
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

void ChannelPanelWidget::clearChannels() {
  channelsList->clear();
  itemByChannelId.clear();
  pendingUnreadByChannelId.clear();
  pendingPreviewByChannelId.clear();
  channelModel->clear();
}