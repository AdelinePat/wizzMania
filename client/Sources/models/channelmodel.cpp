#include "models/channelmodel.hpp"

#include <QString>

ChannelModel::ChannelModel(QObject* parent) : QAbstractListModel(parent) {}

int ChannelModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(channels.size());
}

QVariant ChannelModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(channels.size())) {
    return QVariant();
  }

  const ServerSend::ChannelInfo& channel = channels[index.row()];

  switch (role) {
    case IdRole:
      return QVariant::fromValue(channel.id_channel);
    case TitleRole:
      return QString::fromStdString(channel.title);
    case IsGroupRole:
      return channel.is_group;
    case UnreadCountRole:
      return QVariant::fromValue(channel.unread_count);
    case LastMessageBodyRole:
      return QString::fromStdString(channel.last_message.body);
    case LastMessageTimestampRole:
      return QString::fromStdString(channel.last_message.timestamp);
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> ChannelModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdRole] = "idChannel";
  roles[TitleRole] = "title";
  roles[IsGroupRole] = "isGroup";
  roles[UnreadCountRole] = "unreadCount";
  roles[LastMessageBodyRole] = "lastMessageBody";
  roles[LastMessageTimestampRole] = "lastMessageTimestamp";
  return roles;
}

void ChannelModel::addChannel(const ServerSend::ChannelInfo& channel) {
  beginInsertRows(QModelIndex(), channels.size(), channels.size());
  channels.push_back(channel);
  endInsertRows();
}

void ChannelModel::setChannels(
    const std::vector<ServerSend::ChannelInfo>& chans) {
  beginResetModel();
  channels = chans;
  endResetModel();
}

void ChannelModel::updateChannelUnreadCount(int64_t channelId,
                                            int64_t newCount) {
  for (size_t i = 0; i < channels.size(); ++i) {
    if (channels[i].id_channel == channelId) {
      channels[i].unread_count = newCount;
      QModelIndex idx = index(i);
      emit dataChanged(idx, idx, {UnreadCountRole});
      return;
    }
  }
}

void ChannelModel::clear() {
  beginResetModel();
  channels.clear();
  endResetModel();
}

const ServerSend::ChannelInfo* ChannelModel::getChannelById(int64_t id) const {
  for (const auto& ch : channels) {
    if (ch.id_channel == id) {
      return &ch;
    }
  }
  return nullptr;
}
