#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QString>
#include <QWidget>

#include "services/user_controller.hpp"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget {
  Q_OBJECT

 public:
  explicit LoginWidget(QWidget* parent = nullptr);
  ~LoginWidget();

  void setErrorText(const QString& text);
  void setSuccessText(const QString& text);

 signals:
  void loginSuccessful(const QString& username, const QString& token);
  void registerRequested();

 private slots:
  void onLoginClicked();

 private:
  void sendLoginRequest(const QString& username, const QString& password);
  void setStatusText(const QString& text, bool isError);

  Ui::LoginWidget* ui;
  UserController* userController;
};

#endif  // LOGINWIDGET_H
