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

void ToolSelector::setTools(const quint8 nrows, const quint8 ncols, QMap<quint8, QList<KeyAction>> const& actions, QMap<quint8, QColor> const& colors)
{
    this->nrows = nrows;
    this->ncols = ncols;
    layout = new QGridLayout(this);
    for (QMap<quint8, QList<KeyAction>>::const_iterator it=actions.cbegin(); it!=actions.cend(); it++) {
        ToolButton* button = new ToolButton(*it, colors.value(it.key(), QColor(0,0,0,0)), this);
        buttons[it.key()] = button;
        connect(button, &ToolButton::sendTool, this, &ToolSelector::sendNewTool);
        connect(button, &ToolButton::sendAction, this, &ToolSelector::sendAction);
        layout->addWidget(button, it.key()/16+1, it.key()%16+1);
    }
    setLayout(layout);
}
