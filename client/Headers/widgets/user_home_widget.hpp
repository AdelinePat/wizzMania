#ifndef USERHOMEWIDGET_HPP
#define USERHOMEWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "message_structure.hpp"
#include "widgets/invitation_widget.hpp"

class UserHomeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit UserHomeWidget(QWidget* parent = nullptr);

  void setUsernameCache(const QHash<int64_t, QString>* cache);
  void setIncomingInvitations(
      const std::vector<ServerSend::ChannelInvitation>& invs);
  void setSentInvitations(const std::vector<ServerSend::ChannelInfo>& outs);

 signals:
  void acceptInvitationRequested(int64_t id_channel);
  void rejectInvitationRequested(int64_t id_channel);
  void cancelInvitationRequested(int64_t id_channel);

 private:
  QTabWidget* tabs;
  QListWidget* invitationsList;
  QListWidget* sentInvitationsList;
  const QHash<int64_t, QString>* usernameCache = nullptr;
};

#endif  // USERHOMEWIDGET_HPP
