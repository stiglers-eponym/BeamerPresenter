// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PENSTYLEBUTTON_H
#define PENSTYLEBUTTON_H

#include <memory>

#include "src/config.h"
#include "src/gui/toolpropertybutton.h"
#include "src/preferences.h"

class Tool;

/**
 * @brief Drop down menu for changing the pen style of a draw tool.
 */
class PenStyleButton : public ToolPropertyButton
{
  Q_OBJECT
 public:
  /// Constructor: add all items to the drop down menu.
  PenStyleButton(QWidget *parent = nullptr);

  /// Trivial destructor.
  ~PenStyleButton() {}

 protected:
  /// Set style of tool to selected value.
  void setToolProperty(std::shared_ptr<Tool> tool) const override;

  /// Update currently selected style based on the tool for device.
  void updateTool() override
  {
    toolChanged(preferences()->currentTool(device));
  }

 public slots:
  /// Update currently selected tool property based on tool.
  void toolChanged(std::shared_ptr<const Tool> tool) override;
};

#endif  // PENSTYLEBUTTON_H
