#include "widgets/channelpanelwidget.hpp"

ChannelPanelWidget::ChannelPanelWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(8);

  QLabel* titleLabel = new QLabel("Chats", this);
  titleLabel->setStyleSheet(
      "QLabel { color: rgb(82, 134, 77); padding: 5px; font-size: 14px; "
      "font-weight: 700; }");

  channelsList = new QListWidget(this);
  channelsList->setStyleSheet(
      "QListWidget {"
      "  background-color: rgb(0, 27, 41);"
      "  color: rgb(200, 200, 200);"
      "  border: none;"
      "  border-radius: 4px;"
      "}"
      "QListWidget::item {"
      "  padding: 2px;"
      "  border-bottom: 1px solid rgb(0, 35, 50);"
      "}"
      "QListWidget::item:selected {"
      "  background-color: rgb(82, 134, 77);"
      "}");

  rootLayout->addWidget(titleLabel);
  rootLayout->addWidget(channelsList, 1);

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

    QListWidgetItem* item = new QListWidgetItem(channelsList);
    item->setData(Qt::UserRole, static_cast<qint64>(channel.id_channel));
    item->setData(Qt::UserRole + 1, title);

    ChannelRowWidget* row = new ChannelRowWidget(
        title, preview, static_cast<int>(channel.unread_count),
        channel.is_group, channelsList);
    item->setSizeHint(row->sizeHint());
    channelsList->addItem(item);
    channelsList->setItemWidget(item, row);
  }
}
