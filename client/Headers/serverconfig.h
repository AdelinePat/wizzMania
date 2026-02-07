#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QString>

class ServerConfig {
 public:
  static QString baseUrl();
  static QString loginUrl();
};

#endif  // SERVERCONFIG_H
