#ifndef INCOMINGINVITATIONITEMWIDGET_HPP
#define INCOMINGINVITATIONITEMWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "message_structure.hpp"

class IncomingInvitationItemWidget : public QWidget {
  Q_OBJECT

 public:
  explicit IncomingInvitationItemWidget(
      const ServerSend::ChannelInvitation& inv,
      const QString& inviterName = QString(), QWidget* parent = nullptr);

 signals:
  void acceptClicked(int64_t id_channel);
  void rejectClicked(int64_t id_channel);

 private:
  QLabel* titleLabel;
  QLabel* subLabel;
  QPushButton* acceptBtn;
  QPushButton* rejectBtn;
};

#endif  // INCOMINGINVITATIONITEMWIDGET_HPP
