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

#include "toolselector.h"

void ToolSelector::setTools(const quint8 nrows, const quint8 ncols, QMap<quint8, QList<KeyAction>> const& actionMap, QMap<quint8, FullDrawTool> const& toolMap)
{
    this->nrows = nrows;
    this->ncols = ncols;
    layout = new QGridLayout(this);
    for (auto key : actionMap.keys() + toolMap.keys()) {
        if (buttons.contains(key))
            continue;
        ToolButton* button = new ToolButton(actionMap.value(key), toolMap.value(key, {InvalidTool, QColor(), 0.}), this);
        buttons[key] = button;
        if (button->hasTool()) {
            connect(button, &ToolButton::sendTool, this, &ToolSelector::sendNewTool);
            connect(button, &ToolButton::sendStylusTool, this, &ToolSelector::sendNewStylusTool);
        }
        if (button->hasAction())
            connect(button, &ToolButton::sendAction, this, &ToolSelector::sendAction);
        layout->addWidget(button, key/16+1, key%16+1);
    }
    setLayout(layout);
}
