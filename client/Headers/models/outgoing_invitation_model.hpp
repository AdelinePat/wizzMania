#ifndef OUTGOING_INVITATION_MODEL_H
#define OUTGOING_INVITATION_MODEL_H

#include <QAbstractListModel>
#include <QString>
#include <unordered_map>
#include <vector>

#include "message_structure.hpp"

class OutgoingInvitationModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum OutgoingInvitationRole {
    IdChannelRole        = Qt::UserRole,
    TitleRole            = Qt::UserRole + 1,
    IsGroupRole          = Qt::UserRole + 2,
    ParticipantCountRole = Qt::UserRole + 3,
  };
  Q_ENUM(OutgoingInvitationRole)

  explicit OutgoingInvitationModel(QObject* parent = nullptr);

  // QAbstractListModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // Data manipulation
  void setInvitations(const std::vector<ServerSend::ChannelInfo>& invitations);
  void addInvitation(const ServerSend::ChannelInfo& invitation);
  void removeInvitation(int64_t channelId);
  void clear();

  // Helper
  const ServerSend::ChannelInfo* getInvitationById(int64_t channelId) const;

 private:
  std::vector<ServerSend::ChannelInfo> invitations;
  std::unordered_map<int64_t, int> idToIndex;  // id_channel → index in vector
};

#endif  // OUTGOING_INVITATION_MODEL_H