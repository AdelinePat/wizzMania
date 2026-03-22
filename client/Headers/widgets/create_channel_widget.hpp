#ifndef CREATECHANNELWIDGET_HPP
#define CREATECHANNELWIDGET_HPP

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QVBoxLayout>

#include "utils.hpp"

class CreateChannelWidget : public QDialog {
  Q_OBJECT

 public:
  explicit CreateChannelWidget(QWidget* parent = nullptr);

 signals:
  void createChannelRequested(const QStringList& usernames,
                              const QString& title);

 private:
  void onCreateClicked();
  void setErrorMessage(const QString& message);

  QLineEdit* participantsInput;
  QLineEdit* titleInput;
  QLabel* errorLabel;
  QPushButton* createBtn;
  QPushButton* cancelBtn;
};

#endif  // CREATECHANNELWIDGET_HPP
