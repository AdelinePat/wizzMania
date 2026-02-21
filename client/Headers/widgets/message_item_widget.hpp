#ifndef MESSAGEITEMWIDGET_HPP
#define MESSAGEITEMWIDGET_HPP

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

#include "message_structure.hpp"

class MessageItemWidget : public QWidget {
  Q_OBJECT

 public:
  explicit MessageItemWidget(const ServerSend::Message& message,
                             int64_t currentUserId, const QString& senderName,
                             QWidget* parent = nullptr);
};

#endif  // MESSAGEITEMWIDGET_HPP
