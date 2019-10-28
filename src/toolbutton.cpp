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

static const QMap<KeyAction, QString> actionNames = {
    {DrawMode, "Draw"},
    {ToggleDrawMode, "Draw"},
    {DrawEraser, "Eraser"},
    {DrawPen, "Pen"},
    {DrawHighlighter, "Highlighter"},
    {DrawPointer, "Pointer"},
    {DrawMagnifier, "Magnifier"},
    {DrawTorch, "Torch"},
    {DrawNone, "Hand"},
    {ClearAnnotations, "Clear"},
    {Previous, "prev"},
    {Next, "next"},
    {PlayMultimedia, "play"},
    // TODO: more names, fancy unicode symbols, ...
};

// TODO: use this at other places or do something more efficient (less hard coded lists)
static const QMap<KeyAction, DrawTool> actionToToolMap = {
    {DrawEraser, Eraser},
    {DrawPen, Pen},
    {DrawHighlighter, Highlighter},
    {DrawPointer, Pointer},
    {DrawMagnifier, Magnifier},
    {DrawTorch, Torch},
};

ToolButton::ToolButton(QList<KeyAction> const actions, QColor const color, QWidget* parent) : QPushButton(parent)
{
    QPalette palette;
    this->actions = actions;
    this->color = color;
    if (actions.size() == 0) {
        this->actions = {NoAction};
        return;
    }
    connect(this, &QPushButton::clicked, this, &ToolButton::onClicked);
    if (actions.size() == 1 && actions[0] == DrawPen)
        palette.setColor(QPalette::ButtonText, color);
    else if (color.lightness() > color.alpha()/2)
        palette.setColor(QPalette::Button, color);
    setPalette(palette);
    setText(actionNames.value(actions[0], QString::number(actions[0], 16)));
    if (actions.size() == 1 && color != QColor(0,0,0,0))
        tool = actionToToolMap.value(actions.first(), None);
}

void ToolButton::onClicked()
{
    if (tool == None) {
        for (auto action : actions)
            emit sendAction(action);
    }
    else {
        emit sendTool({tool, color});
        if (actions.size() > 1) {
            for (QList<KeyAction>::const_iterator it=actions.cbegin()+1; it!=actions.cend(); it++)
                emit sendAction(*it);
        }
    }
}
