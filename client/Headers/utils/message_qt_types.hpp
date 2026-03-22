#ifndef MESSAGE_QT_TYPES_H
#define MESSAGE_QT_TYPES_H

#include <QMetaType>

#include "message_structure.hpp"

Q_DECLARE_METATYPE(AuthMessages::WSAuthResponse)
Q_DECLARE_METATYPE(ServerSend::InitialDataResponse)
Q_DECLARE_METATYPE(ServerSend::SendMessageResponse)
Q_DECLARE_METATYPE(ServerSend::ErrorResponse)

#endif  // MESSAGE_QT_TYPES_H