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
#include "drawslide.h"

class ToolSelector : public QWidget
{
    Q_OBJECT
private:
    QPushButton drawMode;
    QPushButton pen1;
    QPushButton pen2;
    QPushButton highlighter;
    QPushButton magnifier;
    QPushButton torch;
    QPushButton pointer;
    QPushButton handTool;
    QPushButton clearAnnotations;
    QPushButton eraser;
    QGridLayout* layout;
    QColor pen1color = QColor("red");
    QColor pen2color = QColor("green");
    QColor highlightercolor = QColor(255,255,0,191);
    QColor pointercolor = QColor(255,0,0,191);
    QColor magnifiercolor = QColor(64,64,64,64);
    QColor torchcolor = QColor(0,0,0,64);

public:
    explicit ToolSelector(QWidget *parent = nullptr);
    ~ToolSelector() {delete layout;}
    void setPenColors(QColor const color1, QColor const color2) {pen1color=color1; pen2color=color2;}
    void setHighlighterColor(QColor const color) {highlightercolor=color;}
    void setPointerColor(QColor const color) {pointercolor=color;}
    void setMagnifierColor(QColor const color) {magnifiercolor=color;}
    void setTorchColor(QColor const color) {torchcolor=color;}

signals:
    void sendNewTool(ColoredDrawTool const tool);
    void sendDrawMode();
    void sendClear();
};

#endif // TOOLSELECTOR_H
