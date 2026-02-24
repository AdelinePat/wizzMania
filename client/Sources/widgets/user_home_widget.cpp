#include "widgets/user_home_widget.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

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

  root->addWidget(tabs);
}

void UserHomeWidget::setIncomingInvitations(
    const std::vector<ServerSend::ChannelInvitation>& invs) {
  invitationsList->clear();
  for (const auto& inv : invs) {
    QString title = QString::fromStdString(inv.title);
    QString inviter = QString::fromStdString("Unknown");
    // if we have inviter info, use it
    // ChannelInvitation has id_inviter and other_participant_ids; inviter name
    // may not be present here — caller should populate user cache if needed

    QListWidgetItem* item = new QListWidgetItem();
    QWidget* row = new QWidget();
    QHBoxLayout* h = new QHBoxLayout(row);
    h->setContentsMargins(4, 4, 4, 4);
    QLabel* lbl = new QLabel(title + " \u2022 From " + inviter, row);
    QPushButton* accept = new QPushButton("Accept", row);
    QPushButton* reject = new QPushButton("Reject", row);
    h->addWidget(lbl, 1);
    h->addWidget(accept);
    h->addWidget(reject);

    // capture id for the lambdas
    int64_t id = inv.id_channel;
    connect(accept, &QPushButton::clicked, this,
            [this, id]() { emit acceptInvitationRequested(id); });
    connect(reject, &QPushButton::clicked, this,
            [this, id]() { emit rejectInvitationRequested(id); });

    item->setSizeHint(row->sizeHint());
    invitationsList->addItem(item);
    invitationsList->setItemWidget(item, row);
  }
}
