#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QString>
#include <QUrl>

class ServerConfig {
 public:
  static QString baseUrl();
  static QString loginUrl();
  static QString webSocketUrl();
};

#endif  // SERVERCONFIG_H