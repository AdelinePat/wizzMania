#include "services/user_controller.hpp"

UserController::UserController(QObject* parent) : QObject(parent) {}

void UserController::login(const QString& username, const QString& password) {
  AuthMessages::LoginRequest req;
  req.username = username.toStdString();
  req.password = password.toStdString();

  QJsonObject payload;
  payload["username"] = QString::fromStdString(req.username);
  payload["password"] = QString::fromStdString(req.password);
  if (username.contains('@')) {
    payload["email"] = username;
  }

  QNetworkReply* reply = api.postJsonWithoutAuth("login", payload);
  connect(reply, &QNetworkReply::finished, this, [this, reply, username]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // TODO move consts after error check, to avoid parsing body if network
    // error? complete range?
    const QByteArray body = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    const QJsonObject obj = doc.object();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      emit loginFailed(api.extractErrorMessage(reply, body));
      reply->deleteLater();
      return;
    }
    //  replace by statuscodeand error code check, to avoid parsing body if
    //  error code is not 200
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
    // also keep id for futur http requests

    emit loginSucceeded(username, token);
    reply->deleteLater();
  });
}

void UserController::logout(const QString& token) {
  QNetworkReply* reply = api.postJsonRequest("logout", QJsonObject(), token);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      // even if server fails, we log out locally
      emit logoutSucceeded();
      reply->deleteLater();
      return;
    }
    emit logoutSucceeded();
    reply->deleteLater();
  });
}

void UserController::registerUser(const QString& username, const QString& email,
                                  const QString& password) {
  QJsonObject payload;
  payload["username"] = username;
  payload["email"] = email;
  payload["password"] = password;

  QNetworkReply* reply = api.postJsonWithoutAuth("register", payload);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    const QJsonObject obj = doc.object();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      emit registerFailed(api.extractErrorMessage(reply, body));
      reply->deleteLater();
      return;
    }

    if (obj.contains("success") && !obj.value("success").toBool()) {
      const QString message = obj.value("message").toString();
      emit registerFailed(message.isEmpty() ? tr("Registration failed.")
                                            : message);
      reply->deleteLater();
      return;
    }

    emit registerSucceeded(tr("You have been registered"));
    reply->deleteLater();
  });
}

void UserController::deleteAccount(const QString& token) {
  QNetworkReply* reply = api.deleteRequest("account", token);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray body = reply->readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(body);
    const QJsonObject obj = doc.object();

    if (reply->error() != QNetworkReply::NoError || statusCode >= 400) {
      emit deleteAccountFailed(api.extractErrorMessage(reply, body));
      reply->deleteLater();
      return;
    }

    const QString successMessage =
        obj.value("message").toString(tr("Account deleted successfully."));
    emit deleteAccountSucceeded(successMessage);
    reply->deleteLater();
  });
}