#ifndef CREATECHANNELWIDGET_HPP
#define CREATECHANNELWIDGET_HPP

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class CreateChannelWidget : public QDialog {
  Q_OBJECT

 public:
  explicit CreateChannelWidget(QWidget* parent = nullptr);

 signals:
  void createChannelRequested(const QStringList& usernames,
                              const QString& title);

 private:
  void onCreateClicked();

  QLineEdit* participantsInput;
  QLineEdit* titleInput;
  QPushButton* createBtn;
  QPushButton* cancelBtn;
};

#endif  // CREATECHANNELWIDGET_HPP
