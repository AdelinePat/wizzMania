#ifndef RIGHTPANELWIDGET_HPP
#define RIGHTPANELWIDGET_HPP

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class RightPanelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit RightPanelWidget(QWidget* parent = nullptr);

  void setChatTitle(const QString& title);
  void clearMessages();
  void addPlainMessage(const QString& text);
  void addMessageWidget(QWidget* widget);
  void setInputEnabled(bool enabled);
  void focusInput();

 signals:
  void sendRequested(const QString& text);
  void wizzRequested();

 private:
  QLabel* titleLabel;
  QListWidget* messagesList;
  QLineEdit* messageInput;
  QPushButton* wizzButton;
  QPushButton* sendButton;
};

#endif  // RIGHTPANELWIDGET_HPP
