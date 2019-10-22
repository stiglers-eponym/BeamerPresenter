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

ToolSelector::ToolSelector(QWidget* parent) : QWidget(parent)
{
    // TODO: fancy labels
    // TODO: use flexible colors
    drawMode.setText("draw");
    QPalette palette;
    palette.setColor(QPalette::ButtonText, pen1color);
    pen1.setPalette(palette);
    pen1.setText("pen");
    palette.setColor(QPalette::ButtonText, pen2color);
    pen2.setPalette(palette);
    pen2.setText("pen");
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::Button, QColor(255,255,0));
    highlighter.setPalette(palette);
    highlighter.setText("highlighter");
    palette.setColor(QPalette::Button, QColor(255,0,0,191));
    pointer.setPalette(palette);
    pointer.setText("pointer");
    torch.setText("torch");
    magnifier.setText("magnifier");
    handTool.setText("hand tool");
    clearAnnotations.setText("clear");
    eraser.setText("eraser");

    layout = new QGridLayout(this);
    layout->addWidget(&drawMode, 1, 1);
    layout->addWidget(&eraser, 1, 2);
    layout->addWidget(&pen1, 1, 3);
    layout->addWidget(&pen2, 1, 4);
    layout->addWidget(&highlighter, 1, 5);
    layout->addWidget(&handTool, 2, 1);
    layout->addWidget(&clearAnnotations, 2, 2);
    layout->addWidget(&pointer, 2, 3);
    layout->addWidget(&torch, 2, 4);
    layout->addWidget(&magnifier, 2, 5);
    setLayout(layout);
    connect(&pen1, &QPushButton::released, this, [&](){sendNewTool({Pen, pen1color});});
    connect(&pen2, &QPushButton::released, this, [&](){sendNewTool({Pen, pen2color});});
    connect(&highlighter, &QPushButton::released, this, [&](){sendNewTool({Highlighter, highlightercolor});});
    connect(&pointer, &QPushButton::released, this, [&](){sendNewTool({Pointer, pointercolor});});
    connect(&torch, &QPushButton::released, this, [&](){sendNewTool({Torch, torchcolor});});
    connect(&magnifier, &QPushButton::released, this, [&](){sendNewTool({Magnifier, magnifiercolor});});
    connect(&eraser, &QPushButton::released, this, [&](){sendNewTool({Eraser, QColor()});});
    connect(&handTool, &QPushButton::released, this, [&](){sendNewTool({None, QColor()});});
    connect(&drawMode, &QPushButton::released, this, &ToolSelector::sendDrawMode);
    connect(&clearAnnotations, &QPushButton::released, this, &ToolSelector::sendClear);
}
