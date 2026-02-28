#include "widgets/user_home_widget.hpp"

#include "widgets/incoming_invitation_item_widget.hpp"
#include "widgets/outgoing_invitation_item_widget.hpp"

UserHomeWidget::UserHomeWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(0, 0, 0, 0);
  root->setSpacing(8);

  // Header with delete account button
  QWidget* header = new QWidget(this);
  QHBoxLayout* headerLayout = new QHBoxLayout(header);
  headerLayout->setContentsMargins(6, 6, 6, 6);

  QLabel* profileLabel = new QLabel("Profile", this);
  profileLabel->setObjectName("profileHeaderLabel");

  QPushButton* deleteAccountBtn = new QPushButton("Delete Account", this);
  deleteAccountBtn->setObjectName("deleteAccountBtn");
  connect(deleteAccountBtn, &QPushButton::clicked, this, [this]() {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Delete Account");
    msgBox.setText("Are you sure you want to delete your account?");
    msgBox.setInformativeText(
        "This action is permanent and cannot be undone. All your data will be "
        "lost.");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.button(QMessageBox::Yes)->setText("Confirm");

    if (msgBox.exec() == QMessageBox::Yes) {
      emit deleteAccountRequested();
    }
  });

  headerLayout->addWidget(profileLabel);
  headerLayout->addStretch();
  headerLayout->addWidget(deleteAccountBtn);

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

  root->addWidget(header);
  root->addWidget(tabs);
}

void UserHomeWidget::setUsernameCache(const QHash<int64_t, QString>* cache) {
  usernameCache = cache;
}

void UserHomeWidget::setModels(IncomingInvitationModel* incomingModel,
                               OutgoingInvitationModel* outgoingModel) {
  this->incomingModel = incomingModel;
  this->outgoingModel = outgoingModel;

  this->setIncomingInvitationModels(incomingModel);
  this->setOutgoingInvitationModels(outgoingModel);
  // if (incomingModel) {
  //   connect(incomingModel, &QAbstractListModel::rowsInserted, this,
  //           &UserHomeWidget::rebuildIncomingInvitations);
  //   connect(incomingModel, &QAbstractListModel::rowsRemoved, this,
  //           &UserHomeWidget::rebuildIncomingInvitations);
  //   connect(incomingModel, &QAbstractListModel::modelReset, this,
  //           &UserHomeWidget::rebuildIncomingInvitations);
  // }

  // rebuildIncomingInvitations();
}

void UserHomeWidget::setOutgoingInvitationModels(
    OutgoingInvitationModel* outgoingModel) {
  if (outgoingModel) {
    connect(outgoingModel, &QAbstractListModel::rowsInserted, this,
            &UserHomeWidget::rebuildOutgoingInvitations);
    connect(outgoingModel, &QAbstractListModel::rowsRemoved, this,
            &UserHomeWidget::rebuildOutgoingInvitations);
    connect(outgoingModel, &QAbstractListModel::modelReset, this,
            &UserHomeWidget::rebuildOutgoingInvitations);
  }
  rebuildOutgoingInvitations();
}

void UserHomeWidget::setIncomingInvitationModels(
    IncomingInvitationModel* incomingModel) {
  this->incomingModel = incomingModel;
  // this->outgoingModel = outgoingModel;

  if (incomingModel) {
    connect(incomingModel, &QAbstractListModel::rowsInserted, this,
            &UserHomeWidget::rebuildIncomingInvitations);
    connect(incomingModel, &QAbstractListModel::rowsRemoved, this,
            &UserHomeWidget::rebuildIncomingInvitations);
    connect(incomingModel, &QAbstractListModel::modelReset, this,
            &UserHomeWidget::rebuildIncomingInvitations);
  }

  // if (outgoingModel) {
  //   connect(outgoingModel, &QAbstractListModel::rowsInserted, this,
  //           &UserHomeWidget::rebuildOutgoingInvitations);
  //   connect(outgoingModel, &QAbstractListModel::rowsRemoved, this,
  //           &UserHomeWidget::rebuildOutgoingInvitations);
  //   connect(outgoingModel, &QAbstractListModel::modelReset, this,
  //           &UserHomeWidget::rebuildOutgoingInvitations);
  // }

  rebuildIncomingInvitations();
  // rebuildOutgoingInvitations();
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
