// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QTabletEvent>
#include <QTouchEvent>
#include <QMouseEvent>
#include "src/log.h"
#include "src/drawing/tool.h"
#include "src/gui/tooldialog.h"
#include "src/gui/toolselectorbutton.h"

ToolSelectorButton::ToolSelectorButton(Tool *tool, QWidget *parent) noexcept
    : ToolButton(tool, parent)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
}


bool ToolSelectorButton::event(QEvent *event) noexcept
{
    if (!tool)
        return QToolButton::event(event);
    switch (event->type())
    {
    case QEvent::TouchBegin:
        event->accept();
        setDown(true);
        return true;
    case QEvent::TouchEnd:
    {
        const QTouchEvent *touchevent = static_cast<const QTouchEvent*>(event);
#if (QT_VERSION_MAJOR >= 6)
        if (touchevent->points().size() != 1 || !rect().contains(touchevent->points().first().position().toPoint()))
#else
        if (touchevent->touchPoints().size() != 1 || !rect().contains(touchevent->touchPoints().first().pos().toPoint()))
#endif
        {
            setDown(false);
            return false;
        }
    }
        [[clang::fallthrough]];
    case QEvent::TabletMove:
    case QEvent::TabletPress:
    case QEvent::MouseButtonPress:
        if (static_cast<QInputEvent*>(event)->modifiers() == Qt::CTRL)
        {
            setTool(ToolDialog::selectTool(tool));
            debug_msg(DebugDrawing, "Changed tool button:" << tool->tool());
            // TODO: save to GUI config
        }
        else
        {
            // If tool doesn't have a device, choose a device based on the input.
            int device = tool->device();
            if (event->type() == QEvent::TabletMove || event->type() == QEvent::TabletPress)
            {
                const QTabletEvent *tablet_event = static_cast<const QTabletEvent*>(event);
                if (tablet_event->pressure() <= 0 || tablet_event->pressure() == 1)
                    break;
                if (device == Tool::NoDevice)
                {
                    device = tablet_event_to_input_device(tablet_event);
                    if (tool->tool() == Tool::Pointer && (device & (Tool::TabletPen | Tool::TabletCursor)))
                        device |= Tool::TabletHover;
                }
            }
            else if (event->type() == QEvent::MouseButtonPress && device == Tool::NoDevice)
            {
                device = static_cast<const QMouseEvent*>(event)->button() << 1;
                if (tool->tool() == Tool::Pointer)
                    device |= Tool::MouseNoButton;
            }
            else if (event->type() == QEvent::TouchEnd && device == Tool::NoDevice)
                device = Tool::TouchInput;

            Tool *newtool = tool->copy();
            newtool->setDevice(device);
            emit sendTool(newtool);
        }
        setDown(false);
        event->accept();
        return true;
    default:
        break;
    }
    return QToolButton::event(event);
}
