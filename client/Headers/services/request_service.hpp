#ifndef RequestService_H
#define RequestService_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QUrl>

#include "utils/path_utils.hpp"

class RequestService : public QObject {
  Q_OBJECT

 public:
  explicit RequestService(QObject* parent = nullptr);
  QNetworkReply* postJsonWithoutAuth(const QString& path,
                                     const QJsonObject& payload);
  QNetworkReply* getRequest(const QString& path, const QString& token);
  QNetworkReply* patchRequest(const QString& path, const QString& token);
  QNetworkReply* patchJsonRequest(const QString& path,
                                  const QJsonObject& payload,
                                  const QString& token);
  QNetworkReply* deleteRequest(const QString& path, const QString& token);
  QNetworkReply* postJsonRequest(const QString& path,
                                 const QJsonObject& payload,
                                 const QString& token);
  QString extractErrorMessage(QNetworkReply* reply,
                              const QByteArray& body) const;

 private:
  QNetworkAccessManager network;
  void SetHeader(QNetworkRequest& request, const QString& path,
                 const QString& token, const QString& ContentType = QString());
};

#endif  // RequestService_H