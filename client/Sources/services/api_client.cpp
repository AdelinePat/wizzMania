#include "services/api_client.hpp"

ApiClient::ApiClient(QObject* parent) : QObject(parent) {}

QNetworkReply* ApiClient::postJson(const QString& path,
                                   const QJsonObject& payload) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  return network.post(request, QJsonDocument(payload).toJson());
}

QNetworkReply* ApiClient::getAuth(const QString& path, const QString& token) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setRawHeader("X-Auth-Token", token.toUtf8());
  return network.get(request);
}

QNetworkReply* ApiClient::patchAuth(const QString& path, const QString& token) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setRawHeader("X-Auth-Token", token.toUtf8());
  return network.sendCustomRequest(request, "PATCH", QByteArray());
}

QNetworkReply* ApiClient::patchJsonAuth(const QString& path,
                                        const QJsonObject& payload,
                                        const QString& token) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("X-Auth-Token", token.toUtf8());
  return network.sendCustomRequest(request, "PATCH",
                                   QJsonDocument(payload).toJson());
}

QNetworkReply* ApiClient::deleteAuth(const QString& path,
                                     const QString& token) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setRawHeader("X-Auth-Token", token.toUtf8());
  return network.sendCustomRequest(request, "DELETE", QByteArray());
}

QNetworkReply* ApiClient::postJsonAuth(const QString& path,
                                       const QJsonObject& payload,
                                       const QString& token) {
  const QString base = ServerConfig::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  QNetworkRequest request{QUrl(urlStr)};
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("X-Auth-Token", token.toUtf8());
  return network.post(request, QJsonDocument(payload).toJson());
}