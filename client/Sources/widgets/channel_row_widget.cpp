#include "widgets/channel_row_widget.hpp"

ChannelRowWidget::ChannelRowWidget(const QString& title, const QString& preview,
                                   int unreadCount, bool isGroup,
                                   QWidget* parent)
    : QWidget(parent) {
  QHBoxLayout* rootLayout = new QHBoxLayout(this);
  rootLayout->setContentsMargins(8, 6, 8, 6);
  rootLayout->setSpacing(8);

  QLabel* iconLabel = new QLabel(isGroup ? "⊞" : "◈", this);
  iconLabel->setAlignment(Qt::AlignCenter);
  iconLabel->setFixedSize(26, 26);
  iconLabel->setStyleSheet(
      "QLabel { background-color: rgb(0, 34, 50); color: rgb(145, 185, 215); "
      "border-radius: 6px; font-size: 12px; }");

  QWidget* textBlock = new QWidget(this);
  QVBoxLayout* textLayout = new QVBoxLayout(textBlock);
  textLayout->setContentsMargins(0, 0, 0, 0);
  textLayout->setSpacing(2);

  QLabel* titleLabel = new QLabel(title, textBlock);
  titleLabel->setStyleSheet(
      "QLabel { color: rgb(220, 230, 240); font-weight: 600; font-size: 12px; "
      "}");

  QLabel* previewLabel = new QLabel(preview, textBlock);
  previewLabel->setStyleSheet(
      "QLabel { color: rgb(135, 150, 165); font-size: 11px; }");
  previewLabel->setWordWrap(false);

  textLayout->addWidget(titleLabel);
  textLayout->addWidget(previewLabel);

  rootLayout->addWidget(iconLabel);
  rootLayout->addWidget(textBlock, 1);

  if (unreadCount > 0) {
    QLabel* unreadLabel = new QLabel(QString::number(unreadCount), this);
    unreadLabel->setAlignment(Qt::AlignCenter);
    unreadLabel->setMinimumWidth(18);
    unreadLabel->setStyleSheet(
        "QLabel { background-color: rgb(82, 134, 77); color: white; "
        "border-radius: 9px; padding: 1px 6px; font-size: 10px; font-weight: "
        "700; }");
    rootLayout->addWidget(unreadLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
  }
}
