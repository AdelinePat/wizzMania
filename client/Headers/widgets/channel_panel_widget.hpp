#ifndef CHANNELPANELWIDGET_HPP
#define CHANNELPANELWIDGET_HPP

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include "message_structure.hpp"
#include "widgets/channel_row_widget.hpp"

class ChannelPanelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChannelPanelWidget(QWidget* parent = nullptr);

  void setChannels(const std::vector<ServerSend::ChannelInfo>& channels);

 signals:
  void channelSelected(int64_t channelId, const QString& title);

 private:
  QListWidget* channelsList;
};

#endif  // CHANNELPANELWIDGET_HPP
