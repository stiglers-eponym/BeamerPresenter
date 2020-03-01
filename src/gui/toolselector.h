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

#ifndef TOOLSELECTOR_H
#define TOOLSELECTOR_H

#include <QPushButton>
#include <QGridLayout>
#include "toolbutton.h"
#include "../slide/drawslide.h"

class ToolSelector : public QWidget
{
    Q_OBJECT
private:
    QMap<quint8, ToolButton*> buttons;
    QGridLayout* layout = nullptr;
    quint8 nrows = 2;
    quint8 ncols = 5;

public:
    explicit ToolSelector(QWidget *parent = nullptr) : QWidget(parent) {}
    ~ToolSelector() {qDeleteAll(buttons); delete layout;}
    void setTools(quint8 const nrows, quint8 const ncols, QMap<quint8, QList<KeyAction>> const& actionMap, QMap<quint8, FullDrawTool> const& toolMap);

signals:
    void sendNewTool(FullDrawTool const& tool);
    void sendAction(KeyAction const action);
};

#endif // TOOLSELECTOR_H
