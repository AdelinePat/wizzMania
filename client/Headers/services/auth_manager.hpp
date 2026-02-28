#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QString>

#include "message_structure.hpp"
#include "services/api_client.hpp"

class AuthManager : public QObject {
  Q_OBJECT

 public:
  explicit AuthManager(QObject* parent = nullptr);

  void login(const QString& username, const QString& password);

  void logout(const QString& token);

 signals:
  void loginSucceeded(const QString& username, const QString& token);
  void loginFailed(const QString& message);

  void logoutSucceeded();
  void logoutFailed(const QString& message);

 private:
  ApiClient api;
};

#endif  // AUTHMANAGER_H