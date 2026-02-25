#ifndef CHANNELROWWIDGET_HPP
#define CHANNELROWWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class ChannelRowWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChannelRowWidget(int64_t channelId, const QString& title,
                            const QString& preview, int unreadCount,
                            bool isGroup, QWidget* parent = nullptr);

 signals:
  void leaveChannelRequested(int64_t channelId);

 private:
  int64_t channelId;
};

#endif  // CHANNELROWWIDGET_HPP
