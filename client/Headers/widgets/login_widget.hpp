#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QString>
#include <QWidget>

#include "services/auth_manager.hpp"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget {
  Q_OBJECT

 public:
  explicit LoginWidget(QWidget* parent = nullptr);
  ~LoginWidget();

 signals:
  void loginSuccessful(const QString& username, const QString& token);
  void registerRequested();

 private slots:
  void onLoginClicked();

 private:
  void sendLoginRequest(const QString& username, const QString& password);
  void setErrorText(const QString& text);

  Ui::LoginWidget* ui;
  AuthManager* authManager;
};

#endif  // LOGINWIDGET_H
