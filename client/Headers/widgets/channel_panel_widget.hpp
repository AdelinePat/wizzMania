#ifndef CHANNELPANELWIDGET_HPP
#define CHANNELPANELWIDGET_HPP

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "message_structure.hpp"
#include "models/channel_model.hpp"
#include "widgets/channel_row_widget.hpp"

class ChannelPanelWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ChannelPanelWidget(QWidget* parent = nullptr);

  void setChannels(const std::vector<ServerSend::ChannelInfo>& channels);
  // Set displayed user (username + initials for avatar)
  void setUserInfo(const QString& username, const QString& initials);
  // Update UI for a channel when a new message arrives: replace preview and
  // optionally increment unread badge.
  void updateChannelOnNewMessage(int64_t channelId, const QString& preview,
                                 bool incrementUnread);
  // Update the unread count for a channel to a specific value (0 to clear, or
  // any positive number to set)
  void updateChannelUnreadCount(int64_t channelId, int unreadCount,
                                int64_t last_id_message);

 signals:
  // Emitted when the user portrait is clicked
  void userHomeRequested();

  void channelSelected(int64_t channelId, const QString& title);

 private:
  QListWidget* channelsList;
  QPushButton* userPortraitBtn;
  QLabel* userPortraitName;
  ChannelModel* channelModel;
  QMap<int64_t, QListWidgetItem*> itemByChannelId;
};

#endif  // CHANNELPANELWIDGET_HPP
