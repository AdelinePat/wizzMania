#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QNetworkAccessManager>
#include <QString>
#include <QWidget>

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

 private slots:
  void onLoginClicked();

 private:
  void sendLoginRequest(const QString& username, const QString& password);
  void handleMockLogin(const QString& username, const QString& password);

  Ui::LoginWidget* ui;
  QNetworkAccessManager* network;
};

#endif  // LOGINWIDGET_H
