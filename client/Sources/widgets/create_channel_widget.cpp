#include "widgets/create_channel_widget.hpp"

CreateChannelWidget::CreateChannelWidget(QWidget* parent) : QDialog(parent) {
  setWindowTitle("New Channel");
  setMinimumWidth(400);

  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(16, 16, 16, 16);
  root->setSpacing(16);

  // Title
  QLabel* titleLabel = new QLabel("Create a New Channel", this);
  titleLabel->setObjectName("createChannelTitleLabel");

  // Description
  QLabel* descLabel =
      new QLabel("Invite people by username to start a conversation", this);
  descLabel->setObjectName("createChannelDescLabel");
  descLabel->setWordWrap(true);

  // Participants section
  QLabel* participantsHeader = new QLabel("Participants", this);
  participantsHeader->setObjectName("createChannelSectionLabel");

  QLabel* participantsDesc = new QLabel(
      "Enter usernames separated by commas (e.g., alice, bob, charlie)", this);
  participantsDesc->setObjectName("createChannelHintLabel");
  participantsDesc->setWordWrap(true);

  participantsInput = new QLineEdit(this);
  participantsInput->setObjectName("createChannelInput");
  participantsInput->setPlaceholderText("username1, username2, username3");

  // Title section
  QLabel* titleSectionLabel = new QLabel("Title (optional)", this);
  titleSectionLabel->setObjectName("createChannelSectionLabel");

  titleInput = new QLineEdit(this);
  titleInput->setObjectName("createChannelInput");
  titleInput->setPlaceholderText("Enter channel title");

  // Action buttons
  QWidget* actionRow = new QWidget(this);
  QHBoxLayout* actionLayout = new QHBoxLayout(actionRow);
  actionLayout->setContentsMargins(0, 0, 0, 0);
  actionLayout->setSpacing(8);

  cancelBtn = new QPushButton("Cancel", this);
  cancelBtn->setObjectName("createChannelCancelBtn");
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

  createBtn = new QPushButton("Create Channel", this);
  createBtn->setObjectName("createChannelCreateBtn");
  connect(createBtn, &QPushButton::clicked, this,
          &CreateChannelWidget::onCreateClicked);

  actionLayout->addStretch();
  actionLayout->addWidget(cancelBtn);
  actionLayout->addWidget(createBtn);

  // Add all to root
  root->addWidget(titleLabel);
  root->addWidget(descLabel);
  root->addWidget(participantsHeader);
  root->addWidget(participantsDesc);
  root->addWidget(participantsInput);
  root->addWidget(titleSectionLabel);
  root->addWidget(titleInput);
  root->addWidget(actionRow);
}

void CreateChannelWidget::onCreateClicked() {
  QString participantsText = participantsInput->text().trimmed();
  if (participantsText.isEmpty()) {
    // Could show an error message here
    return;
  }

  // Split by comma and trim each username
  QStringList usernames;
  QStringList rawList = participantsText.split(',', Qt::SkipEmptyParts);
  for (const QString& username : rawList) {
    QString trimmed = username.trimmed();
    if (!trimmed.isEmpty() && !usernames.contains(trimmed)) {
      usernames.append(trimmed);
    }
  }

  if (usernames.isEmpty()) {
    return;
  }

  QString title = titleInput->text().trimmed();
  emit createChannelRequested(usernames, title);
  accept();  // Close dialog on success
}
