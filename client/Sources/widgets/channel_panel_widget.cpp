#include "widgets/channel_panel_widget.hpp"

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
            item->data(ChannelModel::ChannelRole::IdChannelRole).toString());
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
    const QString title = QString::fromStdString(channel.title);
    const QString preview = QString::fromStdString(channel.last_message.body);

    QListWidgetItem* item = new QListWidgetItem();
    item->setData(ChannelModel::ChannelRole::IdChannelRole,
                  static_cast<qint64>(channel.id_channel));
    item->setData(ChannelModel::ChannelRole::TitleRole, title);
    item->setData(ChannelModel::ChannelRole::LastMessageBodyRole, preview);
    item->setData(ChannelModel::ChannelRole::UnreadCountRole,
                  static_cast<qint64>(channel.unread_count));
    item->setData(ChannelModel::ChannelRole::IsGroupRole, channel.is_group);

    ChannelRowWidget* row = new ChannelRowWidget(
        channel.id_channel, title, preview,
        static_cast<int>(channel.unread_count), channel.is_group, channelsList);
    connect(row, &ChannelRowWidget::leaveChannelRequested, this,
            &ChannelPanelWidget::leaveChannelRequested);
    item->setSizeHint(row->sizeHint());
    channelsList->addItem(item);
    channelsList->setItemWidget(item, row);

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

  // Add to internal model
  channelModel->addChannel(channel);

  const QString title = QString::fromStdString(channel.title);
  const QString preview = QString::fromStdString(channel.last_message.body);

  QListWidgetItem* item = new QListWidgetItem();
  item->setData(ChannelModel::ChannelRole::IdChannelRole,
                static_cast<qint64>(channel.id_channel));
  item->setData(ChannelModel::ChannelRole::TitleRole, title);
  item->setData(ChannelModel::ChannelRole::LastMessageBodyRole, preview);
  item->setData(ChannelModel::ChannelRole::UnreadCountRole,
                static_cast<qint64>(channel.unread_count));
  item->setData(ChannelModel::ChannelRole::IsGroupRole, channel.is_group);

  ChannelRowWidget* row = new ChannelRowWidget(
      channel.id_channel, title, preview,
      static_cast<int>(channel.unread_count), channel.is_group, channelsList);
  connect(row, &ChannelRowWidget::leaveChannelRequested, this,
          &ChannelPanelWidget::leaveChannelRequested);
  item->setSizeHint(row->sizeHint());
  channelsList->addItem(item);
  channelsList->setItemWidget(item, row);

  itemByChannelId[channel.id_channel] = item;

  qInfo() << "[UI][CHANNEL_ADDED] id=" << channel.id_channel
          << " title=" << title;
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
  if (!item) return;

  QString title = item->data(ChannelModel::ChannelRole::TitleRole).toString();
  qint64 unread =
      item->data(ChannelModel::ChannelRole::UnreadCountRole).toLongLong();
  if (incrementUnread) unread += 1;

  // update model
  channelModel->updateLastMessage(channelId, preview);  // ← model stays in sync

  // update item data
  item->setData(ChannelModel::ChannelRole::LastMessageBodyRole, preview);
  item->setData(ChannelModel::ChannelRole::UnreadCountRole, unread);

  bool isGroup = item->data(ChannelModel::ChannelRole::IsGroupRole).toBool();
  ChannelRowWidget* newRow =
      new ChannelRowWidget(channelId, title, preview, static_cast<int>(unread),
                           isGroup, channelsList);
  connect(newRow, &ChannelRowWidget::leaveChannelRequested, this,
          &ChannelPanelWidget::leaveChannelRequested);
  item->setSizeHint(newRow->sizeHint());
  channelsList->setItemWidget(item, newRow);
}
void ChannelPanelWidget::updateChannelUnreadCount(int64_t channelId,
                                                  int unreadCount,
                                                  int64_t last_id_message) {
  QListWidgetItem* item =
      itemByChannelId.value(channelId, nullptr);  // ← one lookup, no loop
  if (!item) return;

  QString title = item->data(ChannelModel::ChannelRole::TitleRole).toString();
  QString preview =
      item->data(ChannelModel::ChannelRole::LastMessageBodyRole).toString();
  bool isGroup = item->data(ChannelModel::ChannelRole::IsGroupRole).toBool();

  // update model
  channelModel->updateChannelUnreadCount(
      channelId, unreadCount, last_id_message);  // ← model stays in sync

  // update item data
  item->setData(ChannelModel::ChannelRole::UnreadCountRole,
                static_cast<qint64>(unreadCount));
  if (last_id_message != 0) {
    item->setData(ChannelModel::ChannelRole::LastReadMessageIdRole,
                  static_cast<qint64>(last_id_message));
  }

  ChannelRowWidget* newRow = new ChannelRowWidget(
      channelId, title, preview, unreadCount, isGroup, channelsList);
  connect(newRow, &ChannelRowWidget::leaveChannelRequested, this,
          &ChannelPanelWidget::leaveChannelRequested);
  item->setSizeHint(newRow->sizeHint());
  channelsList->setItemWidget(item, newRow);
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
  channelModel->clear();
}