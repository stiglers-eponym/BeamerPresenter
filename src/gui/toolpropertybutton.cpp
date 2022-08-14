// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QtConfig>
#include <QMouseEvent>
#include <QTabletEvent>
#include "src/gui/toolpropertybutton.h"
#include "src/drawing/tool.h"
#include "src/preferences.h"

ToolPropertyButton::ToolPropertyButton(QWidget *parent) :
    QComboBox(parent)
{
    setMinimumSize(20, 12);
    setIconSize({16,16});
    setContentsMargins(0,0,0,0);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
#if (QT_VERSION_MAJOR >= 6)
    connect(this, &QComboBox::activated, this, &ToolPropertyButton::changed);
#else
    connect(this, QOverload<int>::of(&QComboBox::activated), this, &ToolPropertyButton::changed);
#endif
}

bool ToolPropertyButton::event(QEvent *event)
{
    const int olddevice = device;
    switch (event->type())
    {
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    {
        const QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->source() != Qt::MouseEventSynthesizedByQt)
            device = mouse_event->buttons() << 1;
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        const QMouseEvent *mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->source() != Qt::MouseEventSynthesizedByQt)
            device = mouse_event->button() << 1;
        break;
    }
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
        device = tablet_event_to_input_device(static_cast<const QTabletEvent*>(event));
        break;
    case QEvent::TouchBegin:
    case QEvent::TouchEnd:
        device = Tool::TouchInput;
        break;
    case QEvent::Resize:
    {
        QSize newsize = size();
        newsize.rwidth() -= 10;
        if (newsize.height() > newsize.width())
            newsize.rheight() = newsize.width();
        else
            newsize.rwidth() = newsize.height();
        setIconSize(newsize);
        break;
    }
    default:
        break;
    }
    if (device != olddevice)
        updateTool();
    return QComboBox::event(event);
}

void ToolPropertyButton::changed(const int index) const
{
    Tool *tool = preferences()->currentTool(device);
    setToolProperty(tool);
}
