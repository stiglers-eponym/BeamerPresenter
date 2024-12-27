// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLWIDGET_H
#define TOOLWIDGET_H

#include <QBoxLayout>
#include <QList>
#include <QWidget>
#include <memory>

#include "src/config.h"
#include "src/drawing/tool.h"

class QSize;
class QResizeEvent;
class Tool;

/**
 * @brief Widget showing which device currently uses which tool
 *
 * This widget shows groups of devices. These groups are mouse devices
 * (left and right mouse button), touch screen, and tablet devices.
 * A group can contain one or more devices. For each device a button
 * shows the currently used tool. Pushing the button opens a dialog
 * for changing the tool.
 */
class ToolWidget : public QWidget
{
  Q_OBJECT

  /// Devices which are currently included in the view.
  Tool::InputDevices devices;
  /// Number of devices shown, used to calculate the layout.
  int total_columns{0};
  /// Direction of the layout.
  const QBoxLayout::Direction direction{QBoxLayout::LeftToRight};
  /// Devices listed in the group for mouse devices.
  QList<Tool::InputDevice> mouse_devices{Tool::MouseLeftButton,
                                         Tool::MouseRightButton};
  /// Devices listed in the group for tablet devices.
  QList<Tool::InputDevice> tablet_devices{Tool::TabletPen, Tool::TabletEraser};

  /// Add a given set of devices as a new group, in an own QFrame.
  void addDeviceGroup(const QList<Tool::InputDevice> &new_devices);

 public:
  /// Constructor: does not add devices, initialize() must be called separately.
  explicit ToolWidget(
      QWidget *parent = nullptr,
      QBoxLayout::Direction direction = QBoxLayout::LeftToRight);

  /// Size hint for layout.
  QSize sizeHint() const noexcept override
  {
    if (direction == QBoxLayout::LeftToRight ||
        direction == QBoxLayout::RightToLeft)
      return {4 + total_columns * 20, 44};
    else
      return {44, 4 + total_columns * 20};
  }

  /// Optimal height depends on width.
  bool hasHeightForWidth() const noexcept override { return true; }

  /// Set the devices included in the group for mouse devices.
  /// Should only be called before initialization.
  void setMouseDevices(QList<Tool::InputDevice> devices)
  {
    mouse_devices = devices;
  }

  /// Set the devices included in the group for tablet devices.
  /// Should only be called before initialization.
  void setTabletDevices(QList<Tool::InputDevice> devices)
  {
    tablet_devices = devices;
  }

  /// Add all devices currently known to Qt
  void initialize();

 protected:
  /// Resize event: tell child buttons to update icons.
  void resizeEvent(QResizeEvent *) override { emit updateIcons(); }

 public slots:
  /// Check if new tool adds a new device. Add that devices if necessary.
  void checkNewTool(std::shared_ptr<const Tool> tool);

 signals:
  /// Tell child buttons to update icons.
  void updateIcons();
};

/// Get icon file name for device.
const char *device_icon(Tool::InputDevice device) noexcept;
/// Get tool tip description for device.
const char *device_description(Tool::InputDevice device) noexcept;

#endif  // TOOLWIDGET_H
