// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QtConfig>
#include <QTabletEvent>
#if (QT_VERSION_MAJOR >= 6)
#include <QPointingDevice>
#endif
#include "src/log.h"
#include "src/drawing/tool.h"

int tablet_event_to_input_device(const QTabletEvent* event)
{
    debug_verbose(DebugDrawing, event << event->pointerType() << event->buttons());
    if (event->pressure() <= 0 && event->type() != QEvent::TabletRelease)
        return Tool::TabletHover;
    switch (event->pointerType())
    {
#if (QT_VERSION_MAJOR >= 6)
    case QPointingDevice::PointerType::Pen:
        if (event->buttons() & Qt::MiddleButton)
            return Tool::TabletMod;
        else
            return Tool::TabletPen;
    case QPointingDevice::PointerType::Eraser:
        return Tool::TabletEraser;
    case QPointingDevice::PointerType::Cursor:
        return Tool::TabletCursor;
    case QPointingDevice::PointerType::Generic:
        return Tool::MouseNoButton;
    case QPointingDevice::PointerType::Finger:
        return Tool::TouchInput;
#else
    case QTabletEvent::Pen:
        return Tool::TabletPen;
    case QTabletEvent::Eraser:
        return Tool::TabletEraser;
    case QTabletEvent::Cursor:
        return Tool::TabletCursor;
    case QTabletEvent::UnknownPointer:
        return Tool::TabletOther;
#endif
    default:
        return Tool::TabletOther;
    }
}


const QString tool_to_description(const Tool::BasicTool tool) noexcept
{
    switch (tool)
    {
    case Tool::Pen:
        return QObject::tr("pen with variable width if the input device supports variable pressure");
    case Tool::FixedWidthPen:
        return QObject::tr("pen with fixed width (independent of input device pressure)");
    case Tool::Eraser:
        return QObject::tr("eraser: deletes drawings");
    case Tool::Highlighter:
        return QObject::tr("highlighter: fixed width drawing which only darkens colors (full color on white background, invisible on black background)");
    case Tool::Pointer:
        return QObject::tr("pointer");
    case Tool::Torch:
        return QObject::tr("torch: darken the slide leaving only a disk unchanged to focus attention on this area");
    case Tool::Magnifier:
        return QObject::tr("enlargen part of the slide");
    case Tool::TextInputTool:
        return QObject::tr("add or edit text on slide");
    case Tool::BasicSelectionTool:
        return QObject::tr("Select objects by clicking on them");
    default:
        return QString();
    };
}
