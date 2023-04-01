// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/toolwidgetbutton.h"
#include "src/gui/tooldialog.h"
#include "src/preferences.h"

ToolWidgetButton::ToolWidgetButton(Tool *tool, const int device, QWidget *parent) noexcept
    : ToolButton(tool, parent), device(device)
{
    connect(this, &ToolWidgetButton::clicked, this, &ToolWidgetButton::selectTool);
}

void ToolWidgetButton::selectTool()
{
    Tool *newtool = ToolDialog::selectTool(tool);
    if (newtool)
    {
        newtool->setDevice(newtool->device() | device);
        emit sendTool(newtool);
    }
}

void ToolWidgetButton::receiveNewTool(const Tool *newtool)
{
    if (!newtool)
        return;
    if (newtool->device() & device)
    {
        Tool *toolcopy = newtool->copy();
        toolcopy->setDevice(device);
        setTool(toolcopy);
    }
    else if (!preferences()->currentTool(device))
        setIcon(QIcon());
}
