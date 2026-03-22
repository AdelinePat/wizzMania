#ifndef UserController_H
#define UserController_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QObject>
#include <QString>

#include "message_structure.hpp"
#include "services/request_service.hpp"

class UserController : public QObject {
  Q_OBJECT

 public:
  explicit UserController(QObject* parent = nullptr);

  void login(const QString& username, const QString& password);
  void registerUser(const QString& username, const QString& email,
                    const QString& password);

  void logout(const QString& token);
  void deleteAccount(const QString& token);

 signals:
  void loginSucceeded(const QString& username, const QString& token);
  void loginFailed(const QString& message);

  void registerSucceeded(const QString& message);
  void registerFailed(const QString& message);

  void logoutSucceeded();
  void logoutFailed(const QString& message);

  void deleteAccountSucceeded(const QString& message);
  void deleteAccountFailed(const QString& message);

 private:
  RequestService api;
};

#endif  // UserController_H