#include "widgets/register_widget.hpp"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "utils.hpp"

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent) {
  QVBoxLayout* root = new QVBoxLayout(this);
  root->setContentsMargins(40, 40, 40, 40);
  root->setSpacing(20);

  // Title
  QLabel* titleLabel = new QLabel("Create Account", this);
  titleLabel->setObjectName("registerTitleLabel");
  titleLabel->setAlignment(Qt::AlignCenter);

  // Username
  QLabel* usernameLabel = new QLabel("Username", this);
  usernameLabel->setObjectName("registerFieldLabel");
  usernameLabel->setAlignment(Qt::AlignCenter);
  usernameInput = new QLineEdit(this);
  usernameInput->setObjectName("registerInput");
  usernameInput->setPlaceholderText("Letters, numbers, underscores only");

  // Email
  QLabel* emailLabel = new QLabel("Email", this);
  emailLabel->setObjectName("registerFieldLabel");
  emailLabel->setAlignment(Qt::AlignCenter);
  emailInput = new QLineEdit(this);
  emailInput->setObjectName("registerInput");
  emailInput->setPlaceholderText("your@email.com");

  // Password
  QLabel* passwordLabel = new QLabel("Password", this);
  passwordLabel->setObjectName("registerFieldLabel");
  passwordLabel->setAlignment(Qt::AlignCenter);
  passwordInput = new QLineEdit(this);
  passwordInput->setObjectName("registerInput");
  passwordInput->setEchoMode(QLineEdit::Password);
  passwordInput->setPlaceholderText(
      "Min 8 chars, uppercase, lowercase, digit, special");

  // Confirm Password
  QLabel* confirmPasswordLabel = new QLabel("Confirm Password", this);
  confirmPasswordLabel->setObjectName("registerFieldLabel");
  confirmPasswordLabel->setAlignment(Qt::AlignCenter);
  confirmPasswordInput = new QLineEdit(this);
  confirmPasswordInput->setObjectName("registerInput");
  confirmPasswordInput->setEchoMode(QLineEdit::Password);
  confirmPasswordInput->setPlaceholderText("Re-enter your password");

  // Error label
  errorLabel = new QLabel(this);
  errorLabel->setObjectName("registerErrorLabel");
  errorLabel->setWordWrap(true);
  errorLabel->setAlignment(Qt::AlignCenter);
  errorLabel->setVisible(false);

  // Buttons
  QWidget* buttonRow = new QWidget(this);
  QHBoxLayout* buttonLayout = new QHBoxLayout(buttonRow);
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(12);

  cancelBtn = new QPushButton("Cancel", this);
  cancelBtn->setObjectName("registerCancelBtn");
  connect(cancelBtn, &QPushButton::clicked, this,
          &RegisterWidget::onCancelClicked);

  createBtn = new QPushButton("Create Account", this);
  createBtn->setObjectName("registerCreateBtn");
  connect(createBtn, &QPushButton::clicked, this,
          &RegisterWidget::onCreateClicked);

  buttonLayout->addWidget(cancelBtn);
  buttonLayout->addWidget(createBtn);

  // Add all to root (centered like login)
  root->addWidget(titleLabel, 0, Qt::AlignHCenter);
  root->addWidget(usernameLabel, 0, Qt::AlignHCenter);
  root->addWidget(usernameInput, 0, Qt::AlignHCenter);
  root->addWidget(emailLabel, 0, Qt::AlignHCenter);
  root->addWidget(emailInput, 0, Qt::AlignHCenter);
  root->addWidget(passwordLabel, 0, Qt::AlignHCenter);
  root->addWidget(passwordInput, 0, Qt::AlignHCenter);
  root->addWidget(confirmPasswordLabel, 0, Qt::AlignHCenter);
  root->addWidget(confirmPasswordInput, 0, Qt::AlignHCenter);
  root->addWidget(errorLabel, 0, Qt::AlignHCenter);
  root->addStretch();
  root->addWidget(buttonRow, 0, Qt::AlignHCenter);
}

RegisterWidget::~RegisterWidget() {}

void RegisterWidget::onCreateClicked() {
  QString username = usernameInput->text().trimmed();
  QString email = emailInput->text().trimmed();
  QString password = passwordInput->text();
  QString confirmPassword = confirmPasswordInput->text();

  setErrorText(QString());

  // Validate username
  if (username.isEmpty()) {
    setErrorText("Please enter a username.");
    usernameInput->setFocus();
    return;
  }

  try {
    std::string cleanedUsername = Utils::clean_username(username.toStdString());
    qInfo() << "[REGISTER] Cleaned username:"
            << QString::fromStdString(cleanedUsername);
  } catch (const BadInput& e) {
    setErrorText(QString::fromStdString(e.what()));
    usernameInput->setFocus();
    return;
  }

  // Validate email
  if (email.isEmpty()) {
    setErrorText("Please enter an email.");
    emailInput->setFocus();
    return;
  }

  if (!Utils::is_valid_email(email.toStdString())) {
    setErrorText("Please enter a valid email address.");
    emailInput->setFocus();
    return;
  }

  // Validate passwords
  if (password.isEmpty()) {
    setErrorText("Please enter a password.");
    passwordInput->setFocus();
    return;
  }

  if (confirmPassword.isEmpty()) {
    setErrorText("Please confirm your password.");
    confirmPasswordInput->setFocus();
    return;
  }

  if (password != confirmPassword) {
    setErrorText("Passwords do not match.");
    passwordInput->setFocus();
    passwordInput->selectAll();
    return;
  }

  if (!Utils::is_valid_password(password.toStdString())) {
    setErrorText(
        "Password must be at least 8 characters and contain uppercase, "
        "lowercase, a digit, and a special character "
        "(!@#$%^&*\\-_=+;:,.<>?/).");
    passwordInput->setFocus();
    passwordInput->selectAll();
    return;
  }

  // All validation passed - log and emit signal
  qInfo() << "[REGISTER] Valid input:"
          << "username=" << username << "email=" << email;
  emit registerRequested(username, email, password);
  clearFields();
}

void RegisterWidget::onCancelClicked() {
  clearFields();
  setErrorText(QString());
  emit cancelRequested();
}

void RegisterWidget::setErrorText(const QString& text) {
  errorLabel->setText(text);
  errorLabel->setVisible(!text.isEmpty());
}

void RegisterWidget::clearFields() {
  usernameInput->clear();
  emailInput->clear();
  passwordInput->clear();
  confirmPasswordInput->clear();
  errorLabel->clear();
  errorLabel->setVisible(false);
}
