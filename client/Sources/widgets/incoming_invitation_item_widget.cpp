#include "widgets/incoming_invitation_item_widget.hpp"

IncomingInvitationItemWidget::IncomingInvitationItemWidget(
    const ServerSend::ChannelInvitation& inv, const QString& inviterName,
    QWidget* parent)
    : QWidget(parent) {
  titleLabel = new QLabel(QString::fromStdString(inv.title), this);
  subLabel = new QLabel(this);
  acceptBtn = new QPushButton("Accept", this);
  rejectBtn = new QPushButton("Reject", this);

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
