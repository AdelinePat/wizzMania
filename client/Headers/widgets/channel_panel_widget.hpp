#ifndef CHANNELPANELWIDGET_HPP
#define CHANNELPANELWIDGET_HPP

#include <QHBoxLayout>
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
  void addChannel(const ServerSend::ChannelInfo& channel);
  bool updateChannel(const ServerSend::ChannelInfo& channel);
  void removeChannel(int64_t channelId);
  const ServerSend::ChannelInfo* getChannelInfo(int64_t channelId) const;
  int unreadCountForChannel(int64_t channelId) const;
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
  void clearChannels();

 signals:
  // Emitted when the user portrait is clicked
  void userHomeRequested();

  void channelSelected(int64_t channelId, const QString& title);

  // Emitted when the create channel (+) button is clicked
  void createChannelRequested();

  // Emitted when the logout button is clicked
  void logoutRequested();

  // Emitted when the leave button on a channel row is clicked
  void leaveChannelRequested(int64_t channelId);

 private:
  void applyChannelDataToItem(QListWidgetItem* item,
                              const ServerSend::ChannelInfo& channel);
  void rebuildChannelRowWidget(QListWidgetItem* item);

  QListWidget* channelsList;
  QPushButton* userPortraitBtn;
  QLabel* userPortraitName;
  QPushButton* createChannelBtn;
  QPushButton* logoutBtn;
  ChannelModel* channelModel;
  QMap<int64_t, QListWidgetItem*> itemByChannelId;
  QMap<int64_t, int> pendingUnreadByChannelId;
  QMap<int64_t, QString> pendingPreviewByChannelId;
};

#endif  // CHANNELPANELWIDGET_HPP
