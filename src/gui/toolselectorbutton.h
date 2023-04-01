// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLSELECTORBUTTON_H
#define TOOLSELECTORBUTTON_H

#include "src/config.h"
#include "src/gui/toolbutton.h"

class ToolSelectorButton : public ToolButton
{
    Q_OBJECT
public:
    ToolSelectorButton(Tool* tool, QWidget *parent = nullptr) noexcept;
    virtual ~ToolSelectorButton() {}
protected:
    /// Emit sendTool based on input event with adjusted device.
    virtual bool event(QEvent *event) noexcept override;
};

#endif // TOOLSELECTORBUTTON_H
