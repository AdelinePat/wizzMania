#include "models/outgoing_invitation_model.hpp"

// #include <QString>

OutgoingInvitationModel::OutgoingInvitationModel(QObject* parent)
    : QAbstractListModel(parent) {}

int OutgoingInvitationModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(invitations.size());
}

QVariant OutgoingInvitationModel::data(const QModelIndex& index,
                                       int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(invitations.size())) {
    return QVariant();
  }

  const ServerSend::ChannelInfo& inv = invitations[index.row()];

  switch (role) {
    case IdChannelRole:
      return QVariant::fromValue(inv.id_channel);
    case TitleRole:
      return QString::fromStdString(inv.title);
    case IsGroupRole:
      return inv.is_group;
    case ParticipantCountRole:
      return static_cast<int>(inv.participants.size());
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> OutgoingInvitationModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdChannelRole] = "idChannel";
  roles[TitleRole] = "title";
  roles[IsGroupRole] = "isGroup";
  roles[ParticipantCountRole] = "participantCount";
  return roles;
}

void OutgoingInvitationModel::setInvitations(
    const std::vector<ServerSend::ChannelInfo>& invs) {
  beginResetModel();
  invitations = invs;
  idToIndex.clear();
  for (int i = 0; i < static_cast<int>(invitations.size()); ++i) {
    idToIndex[invitations[i].id_channel] = i;
  }
  endResetModel();
}

void OutgoingInvitationModel::addInvitation(
    const ServerSend::ChannelInfo& invitation) {
  // if already exists, ignore
  if (idToIndex.count(invitation.id_channel)) return;

  beginInsertRows(QModelIndex(), invitations.size(), invitations.size());
  idToIndex[invitation.id_channel] = static_cast<int>(invitations.size());
  invitations.push_back(invitation);
  endInsertRows();
}

void OutgoingInvitationModel::removeInvitation(int64_t channelId) {
  auto it = idToIndex.find(channelId);
  if (it == idToIndex.end()) return;

  int idx = it->second;

  beginRemoveRows(QModelIndex(), idx, idx);
  invitations.erase(invitations.begin() + idx);

  // rebuild the map since all indices after idx shifted by -1
  idToIndex.clear();
  for (int i = 0; i < static_cast<int>(invitations.size()); ++i) {
    idToIndex[invitations[i].id_channel] = i;
  }
  endRemoveRows();
}

void OutgoingInvitationModel::clear() {
  beginResetModel();
  invitations.clear();
  idToIndex.clear();
  endResetModel();
}

const ServerSend::ChannelInfo* OutgoingInvitationModel::getInvitationById(
    int64_t channelId) const {
  auto it = idToIndex.find(channelId);
  if (it == idToIndex.end()) return nullptr;
  return &invitations[it->second];
}