#ifndef APICLIENT_H
#define APICLIENT_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include "services/serverconfig.hpp"

class ApiClient : public QObject {
  Q_OBJECT

 public:
  explicit ApiClient(QObject* parent = nullptr);

  QNetworkReply* postJson(const QString& path, const QJsonObject& payload);

 private:
  QNetworkAccessManager network;
};

#endif  // APICLIENT_H