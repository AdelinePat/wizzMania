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

QString ApiClient::extractErrorMessage(QNetworkReply* reply,
                                       const QByteArray& body) const {
  const QString fallback =
      reply ? reply->errorString() : QString("Unknown network error");

  const QJsonDocument doc = QJsonDocument::fromJson(body);
  if (doc.isObject()) {
    const QJsonObject obj = doc.object();

    const QString message = obj.value("message").toString();
    if (!message.trimmed().isEmpty()) {
      return message.trimmed();
    }

    const QString error = obj.value("error").toString();
    if (!error.trimmed().isEmpty()) {
      return error.trimmed();
    }

    const QString errorCode = obj.value("error_code").toString();
    if (!errorCode.trimmed().isEmpty()) {
      return errorCode.trimmed();
    }

    if (obj.contains("errors") && obj.value("errors").isArray()) {
      const QJsonArray errors = obj.value("errors").toArray();
      for (const QJsonValue& value : errors) {
        if (value.isString() && !value.toString().trimmed().isEmpty()) {
          return value.toString().trimmed();
        }
      }
    }
  }

  const QString plain = QString::fromUtf8(body).trimmed();
  if (!plain.isEmpty()) {
    return plain;
  }

  return fallback;
}