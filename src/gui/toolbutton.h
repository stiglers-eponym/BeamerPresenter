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

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QDebug>
#include <QPushButton>
#include <QMap>
#include <QIcon>
#include "../enumerates.h"


class ToolButton : public QPushButton
{
    Q_OBJECT

private:
    QColor color;
    DrawTool tool = InvalidTool;
    QList<KeyAction> actions;

public:
    ToolButton(QList<KeyAction> const actions, QColor const color = QColor(0,0,0,0), QWidget* parent = nullptr);
    ColoredDrawTool getDrawTool() {return {tool, color};}

public slots:
    void onClicked();

signals:
    void sendTool(ColoredDrawTool const tool);
    void sendAction(KeyAction const action);
};

#endif // TOOLBUTTON_H
