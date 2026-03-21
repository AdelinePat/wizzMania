#include "services/request_service.hpp"

RequestService::RequestService(QObject* parent) : QObject(parent) {}

QNetworkReply* RequestService::postJsonWithoutAuth(const QString& path,
                                                   const QJsonObject& payload) {
  QNetworkRequest request;
  SetHeader(request, path, QString(""), QString("application/json"));
  return network.post(request, QJsonDocument(payload).toJson());
}

QNetworkReply* RequestService::postJsonRequest(const QString& path,
                                               const QJsonObject& payload,
                                               const QString& token) {
  QNetworkRequest request;
  SetHeader(request, path, token, QString("application/json"));
  return network.post(request, QJsonDocument(payload).toJson());
}
QNetworkReply* RequestService::getRequest(const QString& path,
                                          const QString& token) {
  // const QString base = PathUtils::baseUrl();
  // const QString urlStr =
  //     base.endsWith('/') ? (base + path) : (base + "/" + path);
  // QNetworkRequest request{QUrl(urlStr)};
  // request.setRawHeader("X-Auth-Token", token.toUtf8());
  QNetworkRequest request;
  SetHeader(request, path, token);
  return network.get(request);
}

QNetworkReply* RequestService::patchRequest(const QString& path,
                                            const QString& token) {
  QNetworkRequest request;
  SetHeader(request, path, token);
  return network.sendCustomRequest(request, "PATCH", QByteArray());
}

QNetworkReply* RequestService::patchJsonRequest(const QString& path,
                                                const QJsonObject& payload,
                                                const QString& token) {
  QNetworkRequest request;
  SetHeader(request, path, token, QString("application/json"));
  return network.sendCustomRequest(request, "PATCH",
                                   QJsonDocument(payload).toJson());
}

QNetworkReply* RequestService::deleteRequest(const QString& path,
                                             const QString& token) {
  QNetworkRequest request;
  SetHeader(request, path, token);
  return network.sendCustomRequest(request, "DELETE", QByteArray());
}

QString RequestService::extractErrorMessage(QNetworkReply* reply,
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

void RequestService::SetHeader(QNetworkRequest& request, const QString& path,
                               const QString& token,
                               const QString& ContentType) {
  const QString base = PathUtils::baseUrl();
  const QString urlStr =
      base.endsWith('/') ? (base + path) : (base + "/" + path);
  request.setUrl(QUrl(urlStr));
  if (!ContentType.isEmpty()) {
    request.setHeader(QNetworkRequest::ContentTypeHeader, ContentType);
  }
  if (!token.isEmpty()) {
    request.setRawHeader("X-Auth-Token", token.toUtf8());
  }
}