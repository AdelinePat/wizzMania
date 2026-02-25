#include "widgets/outgoing_invitation_item_widget.hpp"

OutgoingInvitationItemWidget::OutgoingInvitationItemWidget(
    const ServerSend::ChannelInfo& info, QWidget* parent)
    : QWidget(parent) {
  titleLabel = new QLabel(QString::fromStdString(info.title), this);
  titleLabel->setObjectName("invitationTitleLabel");
  subLabel = new QLabel(this);
  subLabel->setObjectName("invitationSubLabel");
  cancelBtn = new QPushButton("Cancel", this);
  cancelBtn->setObjectName("invitationCancelBtn");

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
