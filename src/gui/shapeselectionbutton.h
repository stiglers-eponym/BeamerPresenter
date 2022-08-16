// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SHAPESELECTIONBUTTON_H
#define SHAPESELECTIONBUTTON_H

#include "src/gui/toolpropertybutton.h"

class Tool;

/**
 * @brief Drop down menu for changing the shape property of a draw tool.
 */
class ShapeSelectionButton : public ToolPropertyButton
{
    Q_OBJECT
public:
    /// Constructor: add all items to the drop down menu.
    /// The index of an item is the same as the integer representation
    /// of DrawTool::Shape.
    ShapeSelectionButton(QWidget *parent = NULL);

    /// Trivial destructor.
    ~ShapeSelectionButton() {}

protected:
    /// Set shape of tool to selected value.
    void setToolProperty(Tool* tool) const override;

    /// Update currently selected shape based on the tool for device.
    void updateTool() override;

public slots:
    /// Update currently selected tool property based on tool.
    void toolChanged(Tool *tool) override;
};

#endif // SHAPESELECTIONBUTTON_H
