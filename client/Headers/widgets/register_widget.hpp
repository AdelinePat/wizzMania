#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QWidget>

class RegisterWidget : public QWidget {
  Q_OBJECT

 public:
  explicit RegisterWidget(QWidget* parent = nullptr);
  ~RegisterWidget();

 signals:
  void registerRequested(const QString& username, const QString& email,
                         const QString& password);
  void cancelRequested();

 private slots:
  void onCreateClicked();
  void onCancelClicked();

 private:
  void setErrorText(const QString& text);
  void clearFields();

  QLineEdit* usernameInput;
  QLineEdit* emailInput;
  QLineEdit* passwordInput;
  QLineEdit* confirmPasswordInput;
  QLabel* errorLabel;
  QPushButton* createBtn;
  QPushButton* cancelBtn;
};

#endif  // REGISTERWIDGET_H
