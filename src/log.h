// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef LOG_H
#define LOG_H

#include <QtDebug>

#include "src/config.h"

// Define macros for debugging.

#ifdef QT_DEBUG
#include "src/preferences.h"
// Show debug message if debugging is enabled for this type
#define debug_msg(msg_type, msg) \
  if (preferences()->debug_level & (msg_type)) qDebug() << (msg_type) << msg;
// Show debug message if verbose debugging is enabled for this type
#define debug_verbose(msg_type, msg)                              \
  if ((preferences()->debug_level & (msg_type | DebugVerbose)) == \
      (msg_type | DebugVerbose))                                  \
    qDebug() << (msg_type) << msg;
#else
#define debug_msg(msg_type, msg)
#define debug_verbose(msg_type, msg)
#endif

#endif  // LOG_H
