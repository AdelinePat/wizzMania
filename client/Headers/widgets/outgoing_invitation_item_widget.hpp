#ifndef OUTGOINGINVITATIONITEMWIDGET_HPP
#define OUTGOINGINVITATIONITEMWIDGET_HPP

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "message_structure.hpp"

class OutgoingInvitationItemWidget : public QWidget {
  Q_OBJECT

 public:
  explicit OutgoingInvitationItemWidget(const ServerSend::ChannelInfo& info,
                                        QWidget* parent = nullptr);

 signals:
  void cancelClicked(int64_t id_channel);

 private:
  QLabel* titleLabel;
  QLabel* subLabel;
  QPushButton* cancelBtn;
};

#endif  // OUTGOINGINVITATIONITEMWIDGET_HPP
