#include "serverconfig.h"

#include <QCoreApplication>
#include <QSettings>

QString ServerConfig::baseUrl() {
  const QString configPath =
      QCoreApplication::applicationDirPath() + "/settings.ini";
  QSettings settings(configPath, QSettings::IniFormat);
  return settings.value("server/baseUrl", "http://localhost:8888").toString();
}

QString ServerConfig::loginUrl() {
  const QString base = baseUrl();
  return base.endsWith('/') ? (base + "login") : (base + "/login");
}
