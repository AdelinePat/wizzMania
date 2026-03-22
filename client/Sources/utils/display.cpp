#include "utils/display.hpp"
// Qt version — automatic local timezone conversion
QString DisplayUtils::formatTimestamp(const QString& iso) {
    QDateTime dt = QDateTime::fromString(iso, Qt::ISODate);
    dt = dt.toLocalTime(); // converts UTC → client's OS timezone automatically
    return dt.toString("MMM dd, yyyy 'at' hh:mm");
}