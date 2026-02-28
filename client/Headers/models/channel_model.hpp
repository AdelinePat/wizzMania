#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <unordered_map>
#include <vector>

#include "message_structure.hpp"

class ChannelModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum ChannelRole {
    IdChannelRole = Qt::UserRole,
    TitleRole = Qt::UserRole + 1,
    IsGroupRole = Qt::UserRole + 2,
    UnreadCountRole = Qt::UserRole + 3,
    LastMessageBodyRole = Qt::UserRole + 4,
    LastMessageTimestampRole = Qt::UserRole + 5,
    LastReadMessageIdRole = Qt::UserRole + 6,
  };
  Q_ENUM(ChannelRole)

  explicit ChannelModel(QObject* parent = nullptr);

  // QAbstractListModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // Data manipulation
  void addChannel(const ServerSend::ChannelInfo& channel);
  void removeChannel(int64_t channelId);
  void setChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  bool updateChannelInfo(const ServerSend::ChannelInfo& channel);
  void updateChannelUnreadCount(int64_t channelId, int64_t newCountn,
                                int64_t last_id_message);

  void updateLastMessage(int64_t channelId, const QString& preview);
  // void updateUnreadCount(int64_t channelId, int64_t newCount,
  //                        int64_t lastReadId);
  void clear();

  // Helper
  const ServerSend::ChannelInfo* getChannelById(int64_t id) const;

 private:
  std::vector<ServerSend::ChannelInfo> channels;
  // std::unordered_map<int64_t, ServerSend::ChannelInfo> channelIdToInfos;
  std::unordered_map<int64_t, int>
      idToIndex;  //  get the index from channels vector with id_channel key,
                  //  making the updates in the vector easier
};

#endif  // CHANNELMODEL_H
