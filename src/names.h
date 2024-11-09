// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef NAMES_H
#define NAMES_H

#include <QMap>
#include <QString>

#include "src/config.h"
#include "src/enumerates.h"

/// Convert strings to GuiWidget
GuiWidget string_to_widget_type(const QString &string) noexcept;

/// Translate strings appearing in config to Actions
const QMap<QString, Action> &get_string_to_action() noexcept;

/// Convert string (from configuration file) to gesture
Gesture string_to_gesture(const QString &string) noexcept;

/// Map human readable string to overlay mode.
/// @see PdfMaster
const QMap<QString, OverlayDrawingMode> &get_string_to_overlay_mode() noexcept;

const QMap<PagePart, QString> &get_page_part_names() noexcept;

#ifdef QT_DEBUG
DebugFlag string_to_debug_flag(const QString &string) noexcept;
#endif  // QT_DEBUG

#endif  // NAMES_H
