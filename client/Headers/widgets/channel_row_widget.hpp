#ifndef CHANNELROWWIDGET_HPP
#define CHANNELROWWIDGET_HPP

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

class ChannelRowWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChannelRowWidget(int64_t channelId, const QString& title,
                            const QString& preview, int unreadCount,
                            bool isGroup, QWidget* parent = nullptr);

 protected:
  void resizeEvent(QResizeEvent* event) override;

 signals:
  void leaveChannelRequested(int64_t channelId);

 private:
  void updatePreviewElision();

  int64_t channelId;
  QString previewText;
  QLabel* previewLabel = nullptr;
};

#endif  // CHANNELROWWIDGET_HPP
