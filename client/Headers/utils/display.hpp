// Headers/utils/display.hpp
#ifndef DISPLAY_H
#define DISPLAY_H

#include <QString>
#include <QDateTime> 

class DisplayUtils {
 public:
  static QString formatTimestamp(const QString& iso);

  // future static utility methods go here
};

#endif  // DISPLAY_H