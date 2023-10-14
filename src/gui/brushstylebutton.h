// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef BRUSHSTYLEBUTTON_H
#define BRUSHSTYLEBUTTON_H

#include "src/config.h"
#include "src/gui/toolpropertybutton.h"
#include "src/preferences.h"

class Tool;

/**
 * @brief Drop down menu for changing the brush style of a draw tool.
 */
class BrushStyleButton : public ToolPropertyButton
{
    Q_OBJECT

public:
    /// Constructor: add all items to the drop down menu.
    BrushStyleButton(QWidget *parent = nullptr);

    /// Trivial destructor.
    ~BrushStyleButton() {}

protected:
    /// Set style of tool to selected value.
    void setToolProperty(Tool* tool) const override;

    /// Update currently selected style based on the tool for device.
    void updateTool() override
    {toolChanged(preferences()->currentTool(device));}

public slots:
    /// Update currently selected tool property based on tool.
    void toolChanged(const Tool *tool) override;
};

#endif // BRUSHSTYLEBUTTON_H
