#ifndef USERHOMEWIDGET_HPP
#define USERHOMEWIDGET_HPP

#include <QListWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "message_structure.hpp"

class UserHomeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit UserHomeWidget(QWidget* parent = nullptr);

  void setIncomingInvitations(
      const std::vector<ServerSend::ChannelInvitation>& invs);

 signals:
  void acceptInvitationRequested(int64_t id_channel);
  void rejectInvitationRequested(int64_t id_channel);

 private:
  QTabWidget* tabs;
  QListWidget* invitationsList;
};

#endif  // USERHOMEWIDGET_HPP
