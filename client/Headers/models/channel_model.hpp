#ifndef CHANNELMODEL_H
#define CHANNELMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <vector>

#include "message_structure.hpp"

class ChannelModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum ChannelRole {
    IdRole = Qt::UserRole + 1,
    TitleRole,
    IsGroupRole,
    UnreadCountRole,
    LastMessageBodyRole,
    LastMessageTimestampRole,
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
  void setChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  void updateChannelUnreadCount(int64_t channelId, int64_t newCount);
  void clear();

  // Helper
  const ServerSend::ChannelInfo* getChannelById(int64_t id) const;

 private:
  std::vector<ServerSend::ChannelInfo> channels;
};

#endif  // CHANNELMODEL_H
