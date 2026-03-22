#ifndef PathUtils_H
#define PathUtils_H

#include <QString>
#include <QUrl>

class PathUtils {
 public:
  static QString baseUrl();
  static QString loginUrl();
  static QString webSocketUrl();
};

#endif  // PathUtils_H