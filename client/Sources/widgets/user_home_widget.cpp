#include "widgets/user_home_widget.hpp"

#include "widgets/incoming_invitation_item_widget.hpp"
#include "widgets/outgoing_invitation_item_widget.hpp"

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

void UserHomeWidget::setModels(IncomingInvitationModel* incomingModel,
                               OutgoingInvitationModel* outgoingModel) {
  this->incomingModel = incomingModel;
  this->outgoingModel = outgoingModel;

  if (incomingModel) {
    connect(incomingModel, &QAbstractListModel::rowsInserted, this,
            &UserHomeWidget::onIncomingInvitationsChanged);
    connect(incomingModel, &QAbstractListModel::rowsRemoved, this,
            &UserHomeWidget::onIncomingInvitationsChanged);
    connect(incomingModel, &QAbstractListModel::modelReset, this,
            &UserHomeWidget::onIncomingInvitationsChanged);
  }

  if (outgoingModel) {
    connect(outgoingModel, &QAbstractListModel::rowsInserted, this,
            &UserHomeWidget::onOutgoingInvitationsChanged);
    connect(outgoingModel, &QAbstractListModel::rowsRemoved, this,
            &UserHomeWidget::onOutgoingInvitationsChanged);
    connect(outgoingModel, &QAbstractListModel::modelReset, this,
            &UserHomeWidget::onOutgoingInvitationsChanged);
  }

  rebuildIncomingInvitations();
  rebuildOutgoingInvitations();
}

void UserHomeWidget::onIncomingInvitationsChanged() {
  rebuildIncomingInvitations();
}

void UserHomeWidget::onOutgoingInvitationsChanged() {
  rebuildOutgoingInvitations();
}

void UserHomeWidget::rebuildIncomingInvitations() {
  invitationsList->clear();
  if (!incomingModel) return;

  for (int i = 0; i < incomingModel->rowCount(); ++i) {
    QModelIndex index = incomingModel->index(i);
    int64_t channelId =
        incomingModel->data(index, IncomingInvitationModel::IdChannelRole)
            .toLongLong();
    const ServerSend::ChannelInvitation* inv =
        incomingModel->getInvitationById(channelId);

    if (!inv) continue;

    // Resolve inviter name from cache
    QString inviterName;
    int64_t inviterId =
        incomingModel->data(index, IncomingInvitationModel::InviterIdRole)
            .toLongLong();
    if (usernameCache && usernameCache->contains(inviterId)) {
      inviterName = usernameCache->value(inviterId);
    }

    QListWidgetItem* item = new QListWidgetItem();
    IncomingInvitationItemWidget* w =
        new IncomingInvitationItemWidget(*inv, inviterName);
    connect(w, &IncomingInvitationItemWidget::acceptClicked, this,
            [this](int64_t id) { emit acceptInvitationRequested(id); });
    connect(w, &IncomingInvitationItemWidget::rejectClicked, this,
            [this](int64_t id) { emit rejectInvitationRequested(id); });
    item->setSizeHint(w->sizeHint());
    invitationsList->addItem(item);
    invitationsList->setItemWidget(item, w);
  }
}

void UserHomeWidget::rebuildOutgoingInvitations() {
  sentInvitationsList->clear();
  if (!outgoingModel) return;

  for (int i = 0; i < outgoingModel->rowCount(); ++i) {
    QModelIndex index = outgoingModel->index(i);
    int64_t channelId =
        outgoingModel->data(index, OutgoingInvitationModel::IdChannelRole)
            .toLongLong();
    const ServerSend::ChannelInfo* info =
        outgoingModel->getInvitationById(channelId);

    if (!info) continue;

    QListWidgetItem* item = new QListWidgetItem();
    OutgoingInvitationItemWidget* w = new OutgoingInvitationItemWidget(*info);
    connect(w, &OutgoingInvitationItemWidget::cancelClicked, this,
            [this](int64_t id) { emit cancelInvitationRequested(id); });
    item->setSizeHint(w->sizeHint());
    sentInvitationsList->addItem(item);
    sentInvitationsList->setItemWidget(item, w);
  }
}
