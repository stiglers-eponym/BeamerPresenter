// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#if (QT_VERSION_MAJOR >= 6)
#include <QInputDevice>
#endif
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QImageReader>
#include "src/drawing/tool.h"
#include "src/gui/toolwidgetbutton.h"
#include "src/preferences.h"
#include "src/gui/toolwidget.h"
#include "src/gui/iconlabel.h"
#include "src/log.h"

ToolWidget::ToolWidget(QWidget *parent, Qt::Orientation orientation)
    : QWidget{parent}, orientation{orientation}
{
    debug_msg(DebugWidgets, "Creating ToolWidget");
    connect(this, &ToolWidget::receiveTool, this, &ToolWidget::checkNewTool);
    if (orientation == Qt::Horizontal)
        setLayout(new QHBoxLayout(this));
    else
        setLayout(new QVBoxLayout(this));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    initialize();
}

QSize ToolWidget::sizeHint() const noexcept
{
    if (orientation == Qt::Horizontal)
        return {60+layout()->count()*24, 48};
    else
        return {48, 60+layout()->count()*24};
}

void ToolWidget::initialize()
{
#if (QT_VERSION_MAJOR >= 6)
    const auto all_devices = QInputDevice::devices();
    for (const auto device : all_devices)
    {
        switch (device->type())
        {
        case QInputDevice::DeviceType::Stylus:
        case QInputDevice::DeviceType::Airbrush:
            if ((devices & (Tool::TabletPen | Tool::TabletEraser)) != (Tool::TabletPen | Tool::TabletEraser))
                addDeviceGroup({Tool::TabletPen, Tool::TabletEraser});
            break;
        case QInputDevice::DeviceType::TouchScreen:
            if ((devices & Tool::TouchInput) == 0)
                addDeviceGroup({Tool::TouchInput});
            break;
        case QInputDevice::DeviceType::Mouse:
        case QInputDevice::DeviceType::TouchPad:
            if ((devices & (Tool::MouseLeftButton | Tool::MouseRightButton)) != (Tool::MouseLeftButton | Tool::MouseRightButton))
                addDeviceGroup({Tool::MouseNoButton, Tool::MouseLeftButton, Tool::MouseRightButton});
        default:
            break;
        }
    }
#else
    if ((devices & (Tool::MouseLeftButton | Tool::MouseRightButton)) != (Tool::MouseLeftButton | Tool::MouseRightButton))
        addDeviceGroup({Tool::MouseNoButton, Tool::MouseLeftButton, Tool::MouseRightButton});
#endif
}

void ToolWidget::addDeviceGroup(const QList<Tool::InputDevice> &new_devices)
{
    auto frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    auto grid_layout = new QGridLayout(frame);
    frame->setLayout(grid_layout);
    int used_devices = 0;
    Tool *tool;
    ToolWidgetButton *button;
    QLabel *label;
    for (int device : new_devices)
    {
        if (devices & device)
            continue;
        tool = preferences()->currentTool(device);
        if (tool)
        {
            tool = tool->copy();
            tool->setDevice(device);
        }
        button = new ToolWidgetButton(tool, device, this);
        connect(button, &ToolWidgetButton::sendTool, this, &ToolWidget::sendTool);
        connect(this, &ToolWidget::receiveTool, button, &ToolWidgetButton::receiveNewTool);
        label = new IconLabel(preferences()->icon_path + "/devices/" + device_icon(device) + ".svg", this);
        label->setToolTip(tr(device_description(device)));
        if (orientation == Qt::Horizontal)
        {
            grid_layout->addWidget(button, 1, used_devices);
            grid_layout->addWidget(label, 0, used_devices);
        }
        else
        {
            grid_layout->addWidget(button, used_devices, 1);
            grid_layout->addWidget(label, used_devices, 0);
        }
        devices |= device;
        ++used_devices;
    }
    if (used_devices > 0)
    {
        frame->setMinimumWidth(20*used_devices);
        layout()->addWidget(frame);
    }
    else
        delete frame;
}

const char *device_icon(int device) noexcept
{
    switch (device)
    {
    case Tool::MouseNoButton:
        return "mouse-no-button";
    case Tool::MouseLeftButton:
        return "mouse-left";
    case Tool::MouseMiddleButton:
        return "mouse-middle";
    case Tool::MouseRightButton:
        return "mouse-right";
    case Tool::TabletPen:
    case Tool::TabletCursor:
    case Tool::TabletOther:
        return "tablet-pen";
    case Tool::TabletMod:
        return "";
    case Tool::TabletEraser:
        return "tablet-eraser";
    case Tool::TabletHover:
        return "tablet-hover";
    case Tool::TouchInput:
        return "touch";
    default:
        return "";
    }
}

const char *device_description(int device) noexcept
{
    switch (device)
    {
    case Tool::MouseNoButton:
        return QT_TR_NOOP("mouse pointer, no buttons pressed");
    case Tool::MouseLeftButton:
        return QT_TR_NOOP("left mouse button");
    case Tool::MouseMiddleButton:
        return QT_TR_NOOP("middle mouse button");
    case Tool::MouseRightButton:
        return QT_TR_NOOP("right mouse button");
    case Tool::TabletPen:
    case Tool::TabletCursor:
    case Tool::TabletOther:
        return QT_TR_NOOP("stylus or tablet pen");
    case Tool::TabletMod:
        return QT_TR_NOOP("unsupported tablet device");
    case Tool::TabletEraser:
        return QT_TR_NOOP("eraser of stylus");
    case Tool::TabletHover:
        return QT_TR_NOOP("stylus hover over tablet");
    case Tool::TouchInput:
        return QT_TR_NOOP("touchscreen");
    default:
        return QT_TR_NOOP("unknown devic");
    }
}

void ToolWidget::checkNewTool(const Tool *tool)
{
    if (tool->device() & ~(devices | Tool::MouseNoButton))
#if (QT_VERSION_MAJOR >= 6)
        initialize();
#else
    {
        if ((tool->device() & Tool::TouchInput) && (devices & Tool::TouchInput) == 0)
            addDeviceGroup({Tool::TouchInput});
        else if ((tool->device() & (Tool::TabletPen | Tool::TabletEraser)) && (devices & Tool::TabletPen) == 0)
            addDeviceGroup({Tool::TabletPen, Tool::TabletEraser});
    }
#endif
}
