#include "loginwidget.hpp"

#include "ui_loginwidget.h"

LoginWidget::LoginWidget(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::LoginWidget),
      network(new QNetworkAccessManager(this)) {
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

  sendLoginRequest(username, password);
}

void LoginWidget::sendLoginRequest(const QString& username,
                                   const QString& password) {
  QJsonObject payload;
  payload["username"] = username;
  payload["password"] = password;
  if (username.contains('@')) {
    payload["email"] = username;
  }

  QNetworkRequest request(QUrl(ServerConfig::loginUrl()));
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QNetworkReply* reply =
      network->post(request, QJsonDocument(payload).toJson());
  ui->loginButton->setEnabled(false);

  qDebug() << "Login POST sent to" << request.url().toString() << "for user"
           << username;

  connect(reply, &QNetworkReply::finished, this,
          [this, reply, username, password]() {
            ui->loginButton->setEnabled(true);

            if (reply->error() != QNetworkReply::NoError) {
              qDebug() << "Login request failed:" << reply->errorString();
              QMessageBox::information(
                  this, tr("Login"),
                  tr("Login request sent, but no server response yet.\n"
                     "Mocking login for local testing."));
              handleMockLogin(username, password);
              reply->deleteLater();
              return;
            }

            const QByteArray body = reply->readAll();
            const QJsonDocument doc = QJsonDocument::fromJson(body);
            const QJsonObject obj = doc.object();
            const QString token = obj.value("token").toString();

            if (token.isEmpty()) {
              QMessageBox::warning(this, tr("Login"),
                                   tr("Server response missing token."));
              reply->deleteLater();
              return;
            }

            emit loginSuccessful(username, token);
            reply->deleteLater();
          });
}

void LoginWidget::handleMockLogin(const QString& username,
                                  const QString& password) {
  if (username == "alice" && password == "hash") {
    const QString mockToken = "mock-token-alice";
    QTimer::singleShot(300, this, [this, username, mockToken]() {
      emit loginSuccessful(username, mockToken);
    });
    return;
  }

  QMessageBox::warning(
      this, tr("Login"),
      tr("Mock login only enabled for user 'alice' with password 'hash'."));
}
