#include "services/server_config.hpp"

#include <cstdlib>

#include "utils.hpp"

QString ServerConfig::baseUrl() {
  // Get host from SERVER_HOST env var, default to localhost
  std::string host = Utils::get_env_var("SERVER_HOST", "localhost");
  QString serverHost = QString::fromStdString(host);

  // Get port using Utils function
  uint16_t port = Utils::get_server_port();

  // Determine scheme based on port
  const QString scheme = (port == 443 || port == 8443) ? "https" : "http";
  return scheme + "://" + serverHost + ":" + QString::number(port);
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