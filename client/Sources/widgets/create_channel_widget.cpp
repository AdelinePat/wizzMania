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

  errorLabel = new QLabel(this);
  errorLabel->setObjectName("createChannelErrorLabel");
  errorLabel->setWordWrap(true);
  errorLabel->hide();

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
  root->addWidget(errorLabel);
  root->addWidget(actionRow);

  connect(participantsInput, &QLineEdit::textChanged, this,
          [this]() { setErrorMessage(QString()); });
  connect(titleInput, &QLineEdit::textChanged, this,
          [this]() { setErrorMessage(QString()); });
}

void CreateChannelWidget::onCreateClicked() {
  QString participantsText = participantsInput->text().trimmed();
  if (participantsText.isEmpty()) {
    setErrorMessage("Please enter at least one username.");
    return;
  }

  // Split by comma and sanitize each username with shared utilities.
  QStringList usernames;
  QSet<QString> seen;
  QStringList rawList = participantsText.split(',', Qt::SkipEmptyParts);
  for (const QString& username : rawList) {
    try {
      std::string trimmed = Utils::trim(username.toStdString());
      std::string cleaned = Utils::clean_username(trimmed);
      QString normalized = QString::fromStdString(cleaned);
      if (!normalized.isEmpty() && !seen.contains(normalized)) {
        usernames.append(normalized);
        seen.insert(normalized);
      }
    } catch (const std::exception& e) {
      setErrorMessage(
          QString("Invalid username: %1").arg(QString::fromUtf8(e.what())));
      return;
    }
  }

  if (usernames.isEmpty()) {
    setErrorMessage("Please enter at least one valid username.");
    return;
  }

  QString title;
  try {
    std::string rawTitle = titleInput->text().toStdString();
    if (!rawTitle.empty()) {
      title = QString::fromStdString(Utils::trim(rawTitle));
    }
  } catch (const std::exception&) {
    title.clear();
  }

  setErrorMessage(QString());
  emit createChannelRequested(usernames, title);
  accept();  // Close dialog on success
}

void CreateChannelWidget::setErrorMessage(const QString& message) {
  if (!errorLabel) {
    return;
  }
  if (message.isEmpty()) {
    errorLabel->clear();
    errorLabel->hide();
    return;
  }
  errorLabel->setText(message);
  errorLabel->show();
}
