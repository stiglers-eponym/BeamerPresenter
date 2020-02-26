/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "toolbutton.h"
#include "../names.h"

ToolButton::ToolButton(QList<KeyAction> const actions, FullDrawTool const& tool, QWidget* parent) :
    QPushButton(parent),
    tool(tool),
    actions(actions)
{
    connect(this, &QPushButton::clicked, this, &ToolButton::onClicked);
    QString text;
    QString iconname;
    if (tool.tool == InvalidTool) {
        if (actions.isEmpty())
            return;
        text = actionNames.value(actions[0], QString::number(actions[0], 16));
        iconname = actionIconNames.value(actions[0], "");
    }
    else {
        text = toolNames.value(tool.tool, QString::number(tool.tool, 16));
        iconname = toolIconNames.value(tool.tool, "");
        QPalette palette;
        if (tool.color.lightness() < tool.color.alpha()/3)
            palette.setColor(QPalette::ButtonText, tool.color);
        else
            palette.setColor(QPalette::Button, tool.color);
        setPalette(palette);
    }
    if (iconname.isEmpty())
        setText(text);
    else {
        QIcon const icon = QIcon::fromTheme(iconname);
        if (icon.isNull())
            setText(text);
        else
            setIcon(icon);
    }
#ifdef DISABLE_TOOL_TIP
#else
    setToolTip(text);
#endif
}

void ToolButton::onClicked()
{
    if (tool.tool != InvalidTool)
        emit sendTool(tool);
    for (auto action : actions)
        emit sendAction(action);
}
