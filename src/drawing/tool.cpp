// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QtConfig>
#include <QTabletEvent>
#if (QT_VERSION_MAJOR >= 6)
#include <QPointingDevice>
#endif
#include "src/log.h"
#include "src/drawing/tool.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/selectiontool.h"
#include "src/drawing/texttool.h"

Tool* Tool::copy() const
{
    Tool *newtool;
    if (_tool & Tool::AnyDrawTool)
        newtool = new DrawTool(*static_cast<const DrawTool*>(this));
    else if (_tool & Tool::AnyPointingTool)
        newtool = new PointingTool(*static_cast<const PointingTool*>(this));
    else if (_tool & Tool::AnySelectionTool)
        newtool = new SelectionTool(*static_cast<const SelectionTool*>(this));
    else if (_tool == Tool::TextInputTool)
        newtool = new TextTool(*static_cast<const TextTool*>(this));
    else
        newtool = new Tool(*this);
    newtool->setDevice(_device);
    return newtool;
}

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


const char *tool_to_description(const Tool::BasicTool tool) noexcept
{
    switch (tool)
    {
    case Tool::Pen:
        return QT_TR_NOOP("pen with variable width if the input device supports variable pressure");
    case Tool::FixedWidthPen:
        return QT_TR_NOOP("pen with fixed width (independent of input device pressure)");
    case Tool::Eraser:
        return QT_TR_NOOP("eraser: deletes drawings");
    case Tool::Highlighter:
        return QT_TR_NOOP("highlighter: fixed width drawing which only darkens colors (full color on white background, invisible on black background)");
    case Tool::Pointer:
        return QT_TR_NOOP("pointer");
    case Tool::Torch:
        return QT_TR_NOOP("torch: darken the slide leaving only a disk unchanged to focus attention on this area");
    case Tool::Magnifier:
        return QT_TR_NOOP("enlargen part of the slide");
    case Tool::TextInputTool:
        return QT_TR_NOOP("add or edit text on slide");
    case Tool::BasicSelectionTool:
        return QT_TR_NOOP("Select objects by clicking on them");
    case Tool::RectSelectionTool:
        return QT_TR_NOOP("Select objects in a rectangle");
    case Tool::FreehandSelectionTool:
        return QT_TR_NOOP("Select objects inside a drawn shape");
    default:
        return "";
    };
}
