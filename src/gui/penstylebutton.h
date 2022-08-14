// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PENSTYLEBUTTON_H
#define PENSTYLEBUTTON_H

#include "src/gui/toolpropertybutton.h"
#include "src/preferences.h"

/**
 * @brief Drop down menu for changing the pen style of a draw tool.
 */
class PenStyleButton : public ToolPropertyButton
{
    Q_OBJECT
public:
    /// Constructor: add all items to the drop down menu.
    PenStyleButton(QWidget *parent = NULL);

    /// Trivial destructor.
    ~PenStyleButton() {}

protected:
    /// Set style of tool to selected value.
    void setToolProperty(Tool* tool) const override;

    /// Update currently selected style based on the tool for device.
    void updateTool() override
    {toolChanged(preferences()->currentTool(device));}

public slots:
    /// Update currently selected tool property based on tool.
    void toolChanged(Tool *tool) override;
};

#endif // PENSTYLEBUTTON_H
