#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <vector>

#include "message_structure.hpp"

class MessageModel : public QAbstractListModel {
  Q_OBJECT

 public:
  enum MessageRole {
    IdMessageRole = Qt::UserRole,
    SenderIdRole = Qt::UserRole + 1,
    BodyRole = Qt::UserRole + 2,
    TimestampRole = Qt::UserRole + 3,
    IsSystemRole = Qt::UserRole + 4,
  };
  Q_ENUM(MessageRole)

  explicit MessageModel(QObject* parent = nullptr);

  // QAbstractListModel interface
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  // Data manipulation
  void addMessage(const ServerSend::Message& msg);
  void setMessages(const std::vector<ServerSend::Message>& messages);
  void clear();

 private:
  std::vector<ServerSend::Message> messages;
};

#endif  // MESSAGEMODEL_H
