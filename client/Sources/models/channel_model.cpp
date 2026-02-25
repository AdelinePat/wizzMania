#include "models/channel_model.hpp"

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
    case IdChannelRole:
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
    case LastReadMessageIdRole:
      return QVariant::fromValue(channel.last_read_id_message);
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> ChannelModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdChannelRole] = "idChannelChannel";
  roles[TitleRole] = "title";
  roles[IsGroupRole] = "isGroup";
  roles[UnreadCountRole] = "unreadCount";
  roles[LastMessageBodyRole] = "lastMessageBody";
  roles[LastMessageTimestampRole] = "lastMessageTimestamp";
  roles[LastReadMessageIdRole] = "LastReadMessageId";
  return roles;
}

// void ChannelModel::addChannel(const ServerSend::ChannelInfo& channel) {
//   beginInsertRows(QModelIndex(), channels.size(), channels.size());
//   channels.push_back(channel);
//   endInsertRows();
// }
void ChannelModel::addChannel(const ServerSend::ChannelInfo& channel) {
  beginInsertRows(QModelIndex(), channels.size(), channels.size());
  idToIndex[channel.id_channel] = static_cast<int>(channels.size());
  channels.push_back(channel);
  endInsertRows();
}

// void ChannelModel::setChannels(
//     const std::vector<ServerSend::ChannelInfo>& chans) {
//   beginResetModel();
//   channels = chans;
//   endResetModel();
// }
void ChannelModel::setChannels(
    const std::vector<ServerSend::ChannelInfo>& chans) {
  beginResetModel();
  channels = chans;
  idToIndex.clear();
  for (int i = 0; i < static_cast<int>(channels.size()); ++i) {
    idToIndex[channels[i].id_channel] = i;
  }
  endResetModel();
}

// void ChannelModel::updateChannelUnreadCount(int64_t channelId, int64_t
// newCount,
//                                             int64_t last_id_message) {
//   for (size_t i = 0; i < channels.size(); ++i) {
//     if (channels[i].id_channel == channelId) {
//       channels[i].unread_count = newCount;
//       channels[i].last_read_id_message = last_id_message;
//       QModelIndex idx = index(i);
//       emit dataChanged(idx, idx, {UnreadCountRole});
//       return;
//     }
//   }
// }
void ChannelModel::updateChannelUnreadCount(int64_t channelId, int64_t newCount,
                                            int64_t lastReadId) {
  auto it = idToIndex.find(channelId);
  if (it == idToIndex.end()) return;

  int idx = it->second;
  channels[idx].unread_count = newCount;
  channels[idx].last_read_id_message = lastReadId;

  QModelIndex modelIdx = index(idx);
  emit dataChanged(modelIdx, modelIdx,
                   {UnreadCountRole, LastReadMessageIdRole});
}

// void ChannelModel::clear() {
//   beginResetModel();
//   channels.clear();
//   endResetModel();
// }
void ChannelModel::clear() {
  beginResetModel();
  channels.clear();
  idToIndex.clear();
  endResetModel();
}

// const ServerSend::ChannelInfo* ChannelModel::getChannelById(int64_t id) const
// {
//   for (const auto& ch : channels) {
//     if (ch.id_channel == id) {
//       return &ch;
//     }
//   }
//   return nullptr;
// }

const ServerSend::ChannelInfo* ChannelModel::getChannelById(int64_t id) const {
  auto it = idToIndex.find(id);
  if (it == idToIndex.end()) return nullptr;
  return &channels[it->second];
}

void ChannelModel::updateLastMessage(int64_t channelId,
                                     const QString& preview) {
  auto it = idToIndex.find(channelId);
  if (it == idToIndex.end()) return;

  int idx = it->second;
  channels[idx].last_message.body = preview.toStdString();;

  QModelIndex modelIdx = index(idx);
  emit dataChanged(modelIdx, modelIdx, {LastMessageBodyRole, LastMessageTimestampRole});
}

