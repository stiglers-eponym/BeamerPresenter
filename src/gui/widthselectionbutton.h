// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef WIDTHSELECTIONBUTTON_H
#define WIDTHSELECTIONBUTTON_H

#include <memory>

#include "src/config.h"
#include "src/gui/toolpropertybutton.h"

class Tool;
class QJsonArray;
class QWidget;

/**
 * @brief Drop down menu for changing the width of a draw tool.
 */
class WidthSelectionButton : public ToolPropertyButton
{
  Q_OBJECT

 public:
  /// Constructor: given array should contains numbers defining pen widths.
  WidthSelectionButton(const QJsonArray &array, QWidget *parent = nullptr);
  /// Trivial destructor
  ~WidthSelectionButton() {}

 protected:
  /// Set width of tool to selected value.
  void setToolProperty(std::shared_ptr<Tool> tool) const override;

 public slots:
  /// Update currently selected width based on tool.
  void toolChanged(std::shared_ptr<const Tool> tool) override;
};

#endif  // WIDTHSELECTIONBUTTON_H
