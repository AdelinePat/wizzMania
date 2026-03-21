#include "login_widget.hpp"

#include "ui_login_widget.h"

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoginWidget),
      userController(new UserController(this)) {
  ui->setupUi(this);

  // Connect login button
  connect(ui->loginButton, &QPushButton::clicked, this,
          &LoginWidget::onLoginClicked);

  // Connect register button
  connect(ui->registerButton, &QPushButton::clicked, this,
          &LoginWidget::registerRequested);

  // Allow Enter key to trigger login from password field
  connect(ui->passwordEdit, &QLineEdit::returnPressed, this,
          &LoginWidget::onLoginClicked);
  connect(ui->usernameEdit, &QLineEdit::returnPressed,
          [this]() { ui->passwordEdit->setFocus(); });

  connect(userController, &UserController::loginSucceeded, this,
          [this](const QString& username, const QString& token) {
            ui->loginButton->setEnabled(true);
            setErrorText(QString());
            emit loginSuccessful(username, token);
          });
  connect(userController, &UserController::loginFailed, this,
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
  // TODO CLEAN inputs
  // differencier email & username
  userController->login(username, password);
}

void LoginWidget::setErrorText(const QString& text) {
  setStatusText(text, true);
}

void LoginWidget::setSuccessText(const QString& text) {
  setStatusText(text, false);
}

void LoginWidget::setStatusText(const QString& text, bool isError) {
  if (!ui || !ui->errorLabel) {
    return;
  }

  ui->errorLabel->setWordWrap(true);
  ui->errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

  if (text.isEmpty()) {
    ui->errorLabel->setStyleSheet(QString());
    ui->errorLabel->setMinimumHeight(0);
  } else if (isError) {
    ui->errorLabel->setStyleSheet("color: rgb(220,120,120);");
  } else {
    ui->errorLabel->setStyleSheet("color: rgb(82,134,77);");
  }

  ui->errorLabel->setText(text);
  if (!text.isEmpty()) {
    const int availableWidth =
        ui->errorLabel->width() > 0 ? ui->errorLabel->width() : 300;
    const QRect bounds = ui->errorLabel->fontMetrics().boundingRect(
        QRect(0, 0, availableWidth, 10000), Qt::TextWordWrap | Qt::AlignCenter,
        text);
    ui->errorLabel->setMinimumHeight(bounds.height() + 6);
  }
  ui->errorLabel->setVisible(!text.isEmpty());
}
