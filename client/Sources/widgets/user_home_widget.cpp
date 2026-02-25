#include "widgets/user_home_widget.hpp"

UserHomeWidget::UserHomeWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(8);

  tabs = new QTabWidget(this);

  // Invitations tab
  QWidget* invTab = new QWidget(this);
  QVBoxLayout* invLayout = new QVBoxLayout(invTab);
  invLayout->setContentsMargins(6, 6, 6, 6);
  invitationsList = new QListWidget(invTab);
  invLayout->addWidget(invitationsList);
  tabs->addTab(invTab, "Invitations");

  // Sent invitations tab
  QWidget* sentTab = new QWidget(this);
  QVBoxLayout* sentLayout = new QVBoxLayout(sentTab);
  sentLayout->setContentsMargins(6, 6, 6, 6);
  sentInvitationsList = new QListWidget(sentTab);
  sentLayout->addWidget(sentInvitationsList);
  tabs->addTab(sentTab, "Sent");

  root->addWidget(tabs);
}

void UserHomeWidget::setUsernameCache(const QHash<int64_t, QString>* cache) {
  usernameCache = cache;
}

void UserHomeWidget::setIncomingInvitations(
    const std::vector<ServerSend::ChannelInvitation>& invs) {
  invitationsList->clear();
  for (const auto& inv : invs) {
    // Resolve inviter name from cache
    QString inviterName;
    if (usernameCache && usernameCache->contains(inv.id_inviter)) {
      inviterName = usernameCache->value(inv.id_inviter);
    }

    QListWidgetItem* item = new QListWidgetItem();
    InvitationWidget* w = new InvitationWidget(inv, inviterName);
    connect(w, &InvitationWidget::acceptClicked, this,
            [this](int64_t id) { emit acceptInvitationRequested(id); });
    connect(w, &InvitationWidget::rejectClicked, this,
            [this](int64_t id) { emit rejectInvitationRequested(id); });
    item->setSizeHint(w->sizeHint());
    invitationsList->addItem(item);
    invitationsList->setItemWidget(item, w);
  }
}

void UserHomeWidget::setSentInvitations(
    const std::vector<ServerSend::ChannelInfo>& outs) {
  sentInvitationsList->clear();
  for (const auto& info : outs) {
    QListWidgetItem* item = new QListWidgetItem();
    InvitationWidget* w = new InvitationWidget(info);
    connect(w, &InvitationWidget::cancelClicked, this,
            [this](int64_t id) { emit cancelInvitationRequested(id); });
    item->setSizeHint(w->sizeHint());
    sentInvitationsList->addItem(item);
    sentInvitationsList->setItemWidget(item, w);
  }
}
