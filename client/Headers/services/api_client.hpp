#ifndef APICLIENT_H
#define APICLIENT_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include "services/server_config.hpp"

class ApiClient : public QObject {
  Q_OBJECT

 public:
  explicit ApiClient(QObject* parent = nullptr);

  QNetworkReply* postJson(const QString& path, const QJsonObject& payload);
  QNetworkReply* getAuth(const QString& path, const QString& token);
  QNetworkReply* patchAuth(const QString& path, const QString& token);
  QNetworkReply* patchJsonAuth(const QString& path, const QJsonObject& payload,
                               const QString& token);
  QNetworkReply* deleteAuth(const QString& path, const QString& token);
  QNetworkReply* postJsonAuth(const QString& path, const QJsonObject& payload,
                              const QString& token);
  QString extractErrorMessage(QNetworkReply* reply,
                              const QByteArray& body) const;

 private:
  QNetworkAccessManager network;
};

#endif  // APICLIENT_H