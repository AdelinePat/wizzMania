#ifndef INVITATIONWIDGET_HPP
#define INVITATIONWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "message_structure.hpp"

class InvitationWidget : public QWidget {
  Q_OBJECT

 public:
  explicit InvitationWidget(const ServerSend::ChannelInvitation& inv,
                            const QString& inviterName = QString(),
                            QWidget* parent = nullptr);

  explicit InvitationWidget(const ServerSend::ChannelInfo& info,
                            QWidget* parent = nullptr);

 signals:
  void acceptClicked(int64_t id_channel);
  void rejectClicked(int64_t id_channel);
  void cancelClicked(int64_t id_channel);

 private:
  QLabel* titleLabel;
  QLabel* subLabel;
  QPushButton* acceptBtn;
  QPushButton* rejectBtn;
  QPushButton* cancelBtn;
};

#endif  // INVITATIONWIDGET_HPP
