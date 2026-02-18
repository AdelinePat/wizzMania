#ifndef CHANNELROWWIDGET_HPP
#define CHANNELROWWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

class ChannelRowWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChannelRowWidget(const QString& title, const QString& preview,
                            int unreadCount, bool isGroup,
                            QWidget* parent = nullptr);
};

#endif  // CHANNELROWWIDGET_HPP
