#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class LoginWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

 private slots:
  void onLoginSuccessful(const QString& username);
  void onChatSelected(int row);
  void onSendMessage();

 private:
  void setupChatView();
  void addDemoChats();

  Ui::MainWindow* ui;
  LoginWidget* loginWidget;
  QString currentUser;
};

#endif  // MAINWINDOW_H
