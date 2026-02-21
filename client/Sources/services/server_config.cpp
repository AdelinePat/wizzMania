#include "services/server_config.hpp"

QString ServerConfig::baseUrl() {
  const QString configPath =
      QCoreApplication::applicationDirPath() + "/settings.ini";
  // TODO: stop using .ini for server url. Use common helper to handle .env data
  QSettings settings(configPath, QSettings::IniFormat);
  return settings.value("server/baseUrl", "http://localhost:8888").toString();
}

QString ServerConfig::loginUrl() {
  const QString base = baseUrl();
  return base.endsWith('/') ? (base + "login") : (base + "/login");
}

QString ServerConfig::webSocketUrl() {
  const QUrl base(baseUrl());
  QUrl wsUrl = base;

  const QString scheme = base.scheme().toLower();
  if (scheme == "https") {
    wsUrl.setScheme("wss");
  } else {
    wsUrl.setScheme("ws");
  }

  wsUrl.setPath("/ws");
  wsUrl.setQuery(QString());
  return wsUrl.toString();
}