// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QBoxLayout>
#include <QGridLayout>
#include <QSizePolicy>
#include <QtConfig>
#if (QT_VERSION_MAJOR >= 6)
#include <QInputDevice>
#endif
#include "src/drawing/tool.h"
#include "src/gui/iconlabel.h"
#include "src/gui/toolwidget.h"
#include "src/gui/toolwidgetbutton.h"
#include "src/log.h"
#include "src/master.h"
#include "src/preferences.h"

ToolWidget::ToolWidget(QWidget *parent, QBoxLayout::Direction direction)
    : QWidget{parent}, direction{direction}
{
  debug_msg(DebugWidgets, "Creating ToolWidget");
  connect(master(), &Master::sendNewToolSoft, this, &ToolWidget::checkNewTool);
  setLayout(new QBoxLayout(direction, this));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ToolWidget::initialize()
{
#if (QT_VERSION_MAJOR >= 6)
  const auto all_devices = QInputDevice::devices();
  for (const auto device : all_devices) {
    switch (device->type()) {
      case QInputDevice::DeviceType::Stylus:
      case QInputDevice::DeviceType::Airbrush:
        if ((devices & tablet_devices.constFirst()) == 0)
          addDeviceGroup(tablet_devices);
        break;
      case QInputDevice::DeviceType::TouchScreen:
        if ((devices & Tool::TouchInput) == 0)
          addDeviceGroup({Tool::TouchInput});
        break;
      case QInputDevice::DeviceType::Mouse:
      case QInputDevice::DeviceType::TouchPad:
        if ((devices & mouse_devices.constFirst()) == 0)
          addDeviceGroup(mouse_devices);
      default:
        break;
    }
  }
#else
  if ((devices & (Tool::MouseLeftButton | Tool::MouseRightButton)) !=
      (Tool::MouseLeftButton | Tool::MouseRightButton))
    addDeviceGroup({Tool::MouseLeftButton, Tool::MouseRightButton});
#endif
}

void ToolWidget::addDeviceGroup(const QList<Tool::InputDevice> &new_devices)
{
  auto frame = new QFrame(this);
  frame->setFrameStyle(QFrame::Box | QFrame::Raised);
  auto grid_layout = new QGridLayout(frame);
  frame->setLayout(grid_layout);
  int used_devices = 0;
  ToolWidgetButton *button;
  IconLabel *label;
  for (Tool::InputDevice device : new_devices) {
    if (devices & device) continue;
    std::shared_ptr<Tool> tool = preferences()->currentTool(device);
    if (tool) {
      tool = tool->copy();
      tool->setDevice(device);
    }
    button = new ToolWidgetButton(tool, device, this);
    connect(button, &ToolWidgetButton::sendTool, master(), &Master::setTool,
            Qt::QueuedConnection);
    connect(this, &ToolWidget::updateIcons, button,
            &ToolWidgetButton::updateIcon, Qt::QueuedConnection);
    label = new IconLabel(
        preferences()->icon_path + "/devices/" + device_icon(device) + ".svg",
        this);
    connect(this, &ToolWidget::updateIcons, label, &IconLabel::updateIcon,
            Qt::QueuedConnection);
    label->setToolTip(tr(device_description(device)));
    if (direction == QBoxLayout::LeftToRight ||
        direction == QBoxLayout::RightToLeft) {
      grid_layout->addWidget(button, 1, used_devices);
      grid_layout->addWidget(label, 0, used_devices);
    } else {
      grid_layout->addWidget(button, used_devices, 1);
      grid_layout->addWidget(label, used_devices, 0);
    }
    devices |= device;
    ++used_devices;
    ++total_columns;
  }
  if (used_devices > 0)
    static_cast<QBoxLayout *>(layout())->addWidget(frame, used_devices);
  else
    delete frame;
}

const char *device_icon(Tool::InputDevice device) noexcept
{
  switch (device) {
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
      return "tablet-mod";
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

const char *device_description(Tool::InputDevice device) noexcept
{
  switch (device) {
    case Tool::MouseNoButton:
      return QT_TRANSLATE_NOOP("ToolWidget",
                               "mouse pointer, no buttons pressed");
    case Tool::MouseLeftButton:
      return QT_TRANSLATE_NOOP("ToolWidget", "left mouse button");
    case Tool::MouseMiddleButton:
      return QT_TRANSLATE_NOOP("ToolWidget", "middle mouse button");
    case Tool::MouseRightButton:
      return QT_TRANSLATE_NOOP("ToolWidget", "right mouse button");
    case Tool::TabletPen:
    case Tool::TabletCursor:
    case Tool::TabletOther:
      return QT_TRANSLATE_NOOP("ToolWidget", "stylus or tablet pen");
    case Tool::TabletMod:
      return QT_TRANSLATE_NOOP("ToolWidget", "unsupported tablet device");
    case Tool::TabletEraser:
      return QT_TRANSLATE_NOOP("ToolWidget", "eraser of stylus");
    case Tool::TabletHover:
      return QT_TRANSLATE_NOOP("ToolWidget", "stylus hover over tablet");
    case Tool::TouchInput:
      return QT_TRANSLATE_NOOP("ToolWidget", "touchscreen");
    default:
      return QT_TRANSLATE_NOOP("ToolWidget", "unknown device");
  }
}

void ToolWidget::checkNewTool(std::shared_ptr<const Tool> tool)
{
  if (tool && tool->device() & ~(devices | Tool::MouseNoButton))
#if (QT_VERSION_MAJOR >= 6)
    initialize();
#else
  {
    if ((tool->device() & Tool::TouchInput) &&
        (devices & Tool::TouchInput) == 0)
      addDeviceGroup({Tool::TouchInput});
    else if ((tool->device() & (Tool::TabletPen | Tool::TabletEraser)) &&
             (devices & Tool::TabletPen) == 0)
      addDeviceGroup({Tool::TabletPen, Tool::TabletEraser});
  }
#endif
}
