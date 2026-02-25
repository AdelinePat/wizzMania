#ifndef USERHOMEWIDGET_HPP
#define USERHOMEWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "models/incoming_invitation_model.hpp"
#include "models/outgoing_invitation_model.hpp"

class UserHomeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit UserHomeWidget(QWidget* parent = nullptr);

  void setUsernameCache(const QHash<int64_t, QString>* cache);
  void setModels(IncomingInvitationModel* incomingModel,
                 OutgoingInvitationModel* outgoingModel);

 signals:
  void acceptInvitationRequested(int64_t id_channel);
  void rejectInvitationRequested(int64_t id_channel);
  void cancelInvitationRequested(int64_t id_channel);

 private slots:
  void onIncomingInvitationsChanged();
  void onOutgoingInvitationsChanged();

 private:
  void rebuildIncomingInvitations();
  void rebuildOutgoingInvitations();

  QTabWidget* tabs;
  QListWidget* invitationsList;
  QListWidget* sentInvitationsList;
  const QHash<int64_t, QString>* usernameCache = nullptr;
  IncomingInvitationModel* incomingModel = nullptr;
  OutgoingInvitationModel* outgoingModel = nullptr;
};

#endif  // USERHOMEWIDGET_HPP
