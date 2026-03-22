#include "models/message_model.hpp"

MessageModel::MessageModel(QObject* parent) : QAbstractListModel(parent) {}

int MessageModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return static_cast<int>(messages.size());
}

QVariant MessageModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 ||
      index.row() >= static_cast<int>(messages.size())) {
    return QVariant();
  }

  const ServerSend::Message& msg = messages[index.row()];

  switch (role) {
    case IdMessageRole:
      return QVariant::fromValue(msg.id_message);
    case SenderIdRole:
      return QVariant::fromValue(msg.id_sender);
    case BodyRole:
      return QString::fromStdString(msg.body);
    case TimestampRole:
      return QString::fromStdString(msg.timestamp);
    case IsSystemRole:
      return msg.is_system;
    default:
      return QVariant();
  }
}

QHash<int, QByteArray> MessageModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[IdMessageRole] = "idMessage";
  roles[SenderIdRole] = "senderId";
  roles[BodyRole] = "body";
  roles[TimestampRole] = "timestamp";
  roles[IsSystemRole] = "isSystem";
  return roles;
}

void MessageModel::addMessage(const ServerSend::Message& msg) {
  beginInsertRows(QModelIndex(), messages.size(), messages.size());
  messages.push_back(msg);
  endInsertRows();
}

void MessageModel::setMessages(const std::vector<ServerSend::Message>& msgs) {
  beginResetModel();
  messages = msgs;
  endResetModel();
}

void MessageModel::clear() {
  beginResetModel();
  messages.clear();
  endResetModel();
}
