#include "services/auth_manager.hpp"

AuthManager::AuthManager(QObject* parent) : QObject(parent) {}

void AuthManager::login(const QString& username, const QString& password) {
  AuthMessages::LoginRequest req;
  req.username = username.toStdString();
  req.password = password.toStdString();

  QJsonObject payload;
  payload["username"] = QString::fromStdString(req.username);
  payload["password"] = QString::fromStdString(req.password);
  if (username.contains('@')) {
    payload["email"] = username;
  }

  QNetworkReply* reply = api.postJson("login", payload);
  connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    const QJsonObject obj = doc.object();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      const QString serverMessage = obj.value("message").toString();
      const QString fallback = reply->errorString();
      emit loginFailed(serverMessage.isEmpty() ? fallback : serverMessage);
      reply->deleteLater();
      return;
    }

    if (obj.contains("success") && !obj.value("success").toBool()) {
      emit loginFailed(obj.value("message").toString(tr("Login failed.")));
      reply->deleteLater();
      return;
    }

    const QString token = obj.value("token").toString();
    if (token.isEmpty()) {
      emit loginFailed(tr("Server response missing token."));
      reply->deleteLater();
      return;
    }

    emit loginSucceeded(username, token);
    reply->deleteLater();
  });
}