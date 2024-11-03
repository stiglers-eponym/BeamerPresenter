// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLWIDGETBUTTON_H
#define TOOLWIDGETBUTTON_H

#include <memory>

#include "src/config.h"
#include "src/gui/toolbutton.h"

class Tool;

/**
 * @brief Button showing a tool in ToolWidget.
 *
 * Shows a tool which is currently connected to a fixed input device.
 * Clicking the button opens a dialog to change the tool.
 *
 * @inherits ToolButton
 * @see ToolWidget
 */
class ToolWidgetButton : public ToolButton
{
  Q_OBJECT

  /// Input device, for which the tool is shown.
  const int device;

 public:
  /// Almost trivial constructor.
  ToolWidgetButton(std::shared_ptr<Tool> tool, const int device,
                   QWidget *parent = nullptr) noexcept;
  /// Trivial desctructor.
  virtual ~ToolWidgetButton() {}

 protected slots:
  /// Select a new tool using ToolDialog.
  virtual void selectTool();

 public slots:
  /// Receive a new tool from master. Only changes the tool if the
  /// new tool is connected to this->device.
  void receiveNewTool(std::shared_ptr<Tool> newtool);
};

#endif  // TOOLWIDGETBUTTON_H
