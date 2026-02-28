#include "models/incoming_invitation_model.hpp"

// #include <QString>

IncomingInvitationModel::IncomingInvitationModel(QObject* parent)
    : QAbstractListModel(parent) {}

int IncomingInvitationModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(invitations.size());
}

QVariant IncomingInvitationModel::data(const QModelIndex& index,
                                       int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(invitations.size())) {
    return QVariant();
  }

  const ServerSend::ChannelInvitation& inv = invitations[index.row()];

  switch (role) {
    case IdChannelRole:
      return QVariant::fromValue(inv.id_channel);
    case TitleRole:
      return QString::fromStdString(inv.title);
    case InviterIdRole:
      return QVariant::fromValue(inv.id_inviter);
    case ParticipantCountRole:
      return static_cast<int>(inv.other_participant_ids.size());
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> IncomingInvitationModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdChannelRole]        = "idChannel";
  roles[TitleRole]            = "title";
  roles[InviterIdRole]        = "inviterId";
  roles[ParticipantCountRole] = "participantCount";
  return roles;
}

void IncomingInvitationModel::setInvitations(
    const std::vector<ServerSend::ChannelInvitation>& invs) {
  beginResetModel();
  invitations = invs;
  idToIndex.clear();
  for (int i = 0; i < static_cast<int>(invitations.size()); ++i) {
    idToIndex[invitations[i].id_channel] = i;
  }
  endResetModel();
}

void IncomingInvitationModel::addInvitation(
    const ServerSend::ChannelInvitation& invitation) {
  // if already exists, ignore
  if (idToIndex.count(invitation.id_channel)) return;

  beginInsertRows(QModelIndex(), invitations.size(), invitations.size()); // TODO KESAKO beginInsertRows ??
  idToIndex[invitation.id_channel] = static_cast<int>(invitations.size()); // TODO : need to rebuild entire map or not????
  invitations.push_back(invitation);
  endInsertRows();
}

void IncomingInvitationModel::removeInvitation(int64_t channelId) {
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

void IncomingInvitationModel::clear() {
  beginResetModel();
  invitations.clear();
  idToIndex.clear();
  endResetModel();
}

const ServerSend::ChannelInvitation* IncomingInvitationModel::getInvitationById(
    int64_t channelId) const {
  auto it = idToIndex.find(channelId);
  if (it == idToIndex.end()) return nullptr;
  return &invitations[it->second];
}