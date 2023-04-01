// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QInputDevice>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QImageReader>
#include "src/drawing/tool.h"
#include "src/gui/toolwidgetbutton.h"
#include "src/preferences.h"
#include "src/gui/toolwidget.h"
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
    const auto all_devices = QInputDevice::devices();
    debug_msg(DebugWidgets, "found devices:" << all_devices);
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
        label = new QLabel(this);
        QImageReader reader(preferences()->icon_path + "/devices/" + get_device_icon(device));
        reader.setScaledSize({32,32});
        if (reader.canRead())
            label->setPixmap(QPixmap::fromImageReader(&reader));
        else
            label->setText("?");
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
        layout()->addWidget(frame);
    else
        delete frame;
}

const QString get_device_icon(int device) noexcept
{
    switch (device)
    {
    case Tool::MouseNoButton:
        return "";
    case Tool::MouseLeftButton:
        return "";
    case Tool::MouseMiddleButton:
        return "";
    case Tool::MouseRightButton:
        return "";
    case Tool::TabletPen:
    case Tool::TabletOther:
        return "";
    case Tool::TabletMod:
        return "";
    case Tool::TabletEraser:
        return "";
    case Tool::TabletHover:
        return "";
    case Tool::TouchInput:
        return "";
    default:
        return "";
    }
}

void ToolWidget::checkNewTool(const Tool *tool)
{
    if (tool->device() & ~(devices | Tool::MouseNoButton))
        initialize();
}
