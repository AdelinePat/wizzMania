#include "loginwidget.h"

#include <QMessageBox>

#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::LoginWidget) {
  ui->setupUi(this);

  // Connect login button
  connect(ui->loginButton, &QPushButton::clicked, this,
          &LoginWidget::onLoginClicked);

  // Allow Enter key to trigger login from password field
  connect(ui->passwordEdit, &QLineEdit::returnPressed, this,
          &LoginWidget::onLoginClicked);
  connect(ui->usernameEdit, &QLineEdit::returnPressed,
          [this]() { ui->passwordEdit->setFocus(); });
}

LoginWidget::~LoginWidget() { delete ui; }

void LoginWidget::onLoginClicked() {
  QString username = ui->usernameEdit->text().trimmed();
  QString password = ui->passwordEdit->text();

  if (username.isEmpty()) {
    QMessageBox::warning(this, tr("Login"), tr("Please enter your username."));
    ui->usernameEdit->setFocus();
    return;
  }

  if (password.isEmpty()) {
    QMessageBox::warning(this, tr("Login"), tr("Please enter your password."));
    ui->passwordEdit->setFocus();
    return;
  }

  // For now, accept any non-empty credentials (no server communication yet)
  // Later this will connect to the server for authentication
  emit loginSuccessful(username);
}
