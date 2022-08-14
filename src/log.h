#ifndef LOG_H
#define LOG_H

#include "src/config.h"
#include "src/preferences.h"

// Define macros for debugging.

#ifdef QT_DEBUG
// Show warning if debugging is enabled
#define warn_msg(msg) qWarning() << msg
// Show debug message if debugging is enabled for this type
#define debug_msg(msg_type, msg) if (preferences()->debug_level & (msg_type)) qDebug() << (msg_type) << msg;
// Show debug message if verbose debugging is enabled for this type
#define debug_verbose(msg_type, msg) if ((preferences()->debug_level & (msg_type|DebugVerbose)) == (msg_type|DebugVerbose)) qDebug() << (msg_type) << msg;
#else
#define debug_msg(msg_type, msg)
#define debug_verbose(msg_type, msg)
#define warn_msg(msg) qWarning() << msg
#endif


#endif // LOG_H
