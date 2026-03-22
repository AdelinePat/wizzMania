#ifndef CHANNELROWWIDGET_HPP
#define CHANNELROWWIDGET_HPP

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
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

 private:
  void updatePreviewElision();

  int64_t channelId;
  QString previewText;
  QLabel* previewLabel = nullptr;
};

#endif  // CHANNELROWWIDGET_HPP
