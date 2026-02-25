#include "widgets/invitation_widget.hpp"

InvitationWidget::InvitationWidget(const ServerSend::ChannelInvitation& inv,
                                   const QString& inviterName, QWidget* parent)
    : QWidget(parent) {
  titleLabel = new QLabel(QString::fromStdString(inv.title), this);
  subLabel = new QLabel(this);
  acceptBtn = new QPushButton("Accept", this);
  rejectBtn = new QPushButton("Reject", this);
  cancelBtn = nullptr;

  // Build subtitle: show inviter and other participants
  QString inviter = inviterName.isEmpty()
                        ? QString("User %1").arg(inv.id_inviter)
                        : inviterName;

  QStringList otherNames;
  for (const auto& p : inv.other_participant_ids) {
    otherNames.append(QString::fromStdString(p.username));
  }

  QString subtitle = "From " + inviter;
  if (!otherNames.isEmpty()) {
    subtitle += " · " + otherNames.join(", ");
  }
  subLabel->setText(subtitle);

  QHBoxLayout* h = new QHBoxLayout(this);
  h->setContentsMargins(4, 4, 4, 4);
  h->addWidget(titleLabel, 1);
  h->addWidget(subLabel);
  h->addWidget(acceptBtn);
  h->addWidget(rejectBtn);

  int64_t id = inv.id_channel;
  connect(acceptBtn, &QPushButton::clicked, this,
          [this, id]() { emit acceptClicked(id); });
  connect(rejectBtn, &QPushButton::clicked, this,
          [this, id]() { emit rejectClicked(id); });
}

InvitationWidget::InvitationWidget(const ServerSend::ChannelInfo& info,
                                   QWidget* parent)
    : QWidget(parent) {
  titleLabel = new QLabel(QString::fromStdString(info.title), this);
  subLabel = new QLabel(this);
  acceptBtn = nullptr;
  rejectBtn = nullptr;
  cancelBtn = new QPushButton("Cancel", this);

  // Display participants by name
  QStringList participantNames;
  for (const auto& p : info.participants) {
    participantNames.append(QString::fromStdString(p.username));
  }

  QString subtitle;
  if (participantNames.isEmpty()) {
    subtitle = "Pending · No participants";
  } else if (participantNames.size() == 1) {
    subtitle = "Waiting for " + participantNames.first();
  } else {
    subtitle = "Waiting for " + participantNames.join(", ");
  }
  subLabel->setText(subtitle);

  QHBoxLayout* h = new QHBoxLayout(this);
  h->setContentsMargins(4, 4, 4, 4);
  h->addWidget(titleLabel, 1);
  h->addWidget(subLabel);
  h->addWidget(cancelBtn);

  int64_t id = info.id_channel;
  connect(cancelBtn, &QPushButton::clicked, this,
          [this, id]() { emit cancelClicked(id); });
}
