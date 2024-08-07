// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLSELECTORBUTTON_H
#define TOOLSELECTORBUTTON_H

#include <memory>

#include "src/config.h"
#include "src/gui/toolbutton.h"

class Tool;
class QEvent;
class QJsonArray;

/**
 * @brief Button showing a tool in ToolSelectorWidget
 *
 * This button shows a tool. Clicking the button will either set the
 * tool for the device(s) defined in tool, or set the tool for the
 * device used to click the button.
 *
 * @see ToolSelectorWidget
 */
class ToolSelectorButton : public ToolButton
{
  Q_OBJECT

 public:
  /// Almost trivial constructor.
  ToolSelectorButton(std::shared_ptr<Tool> tool,
                     QWidget *parent = nullptr) noexcept;

  /// Trivial destructor.
  virtual ~ToolSelectorButton() {}

 protected:
  /// Emit sendTool based on input event with adjusted device.
  virtual bool event(QEvent *event) noexcept override;
};

#endif  // TOOLSELECTORBUTTON_H
