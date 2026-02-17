#include "loginwidget.hpp"

#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoginWidget),
      authManager(new AuthManager(this)) {
  ui->setupUi(this);

  // Connect login button
  connect(ui->loginButton, &QPushButton::clicked, this,
          &LoginWidget::onLoginClicked);

  // Allow Enter key to trigger login from password field
  connect(ui->passwordEdit, &QLineEdit::returnPressed, this,
          &LoginWidget::onLoginClicked);
  connect(ui->usernameEdit, &QLineEdit::returnPressed,
          [this]() { ui->passwordEdit->setFocus(); });

  connect(authManager, &AuthManager::loginSucceeded, this,
          [this](const QString& username, const QString& token) {
            ui->loginButton->setEnabled(true);
            setErrorText(QString());
            emit loginSuccessful(username, token);
          });
  connect(authManager, &AuthManager::loginFailed, this,
          [this](const QString& message) {
            ui->loginButton->setEnabled(true);
            setErrorText(message);
          });
}

LoginWidget::~LoginWidget() { delete ui; }

void LoginWidget::onLoginClicked() {
  QString username = ui->usernameEdit->text().trimmed();
  QString password = ui->passwordEdit->text();
  setErrorText(QString());

  if (username.isEmpty()) {
    setErrorText(tr("Please enter your username."));
    ui->usernameEdit->setFocus();
    return;
  }

  if (password.isEmpty()) {
    setErrorText(tr("Please enter your password."));
    ui->passwordEdit->setFocus();
    return;
  }

  sendLoginRequest(username, password);
}

void LoginWidget::sendLoginRequest(const QString& username,
                                   const QString& password) {
  ui->loginButton->setEnabled(false);
  setErrorText(QString());
  authManager->login(username, password);
}

void LoginWidget::setErrorText(const QString& text) {
  if (!ui || !ui->errorLabel) {
    return;
  }
  ui->errorLabel->setText(text);
  ui->errorLabel->setVisible(!text.isEmpty());
}
