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
    drawMode.setText("draw");
    QPalette palette;
    palette.setColor(QPalette::ButtonText, QColor(255,0,0));
    redPen.setPalette(palette);
    redPen.setText("red");
    palette.setColor(QPalette::ButtonText, QColor(0,191,0));
    greenPen.setPalette(palette);
    greenPen.setText("green");
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

    layout = new QGridLayout(this);
    layout->addWidget(&drawMode, 1, 1);
    layout->addWidget(&redPen, 1, 2);
    layout->addWidget(&greenPen, 1, 3);
    layout->addWidget(&highlighter, 1, 4);
    layout->addWidget(&handTool, 2, 1);
    layout->addWidget(&pointer, 2, 2);
    layout->addWidget(&torch, 2, 3);
    layout->addWidget(&magnifier, 2, 4);
    setLayout(layout);
    connect(&redPen, &QPushButton::released, this, [&](){sendNewTool(RedPen);});
    connect(&greenPen, &QPushButton::released, this, [&](){sendNewTool(GreenPen);});
    connect(&highlighter, &QPushButton::released, this, [&](){sendNewTool(Highlighter);});
    connect(&pointer, &QPushButton::released, this, [&](){sendNewTool(Pointer);});
    connect(&torch, &QPushButton::released, this, [&](){sendNewTool(Torch);});
    connect(&magnifier, &QPushButton::released, this, [&](){sendNewTool(Magnifier);});
    connect(&handTool, &QPushButton::released, this, [&](){sendNewTool(None);});
    connect(&drawMode, &QPushButton::released, this, &ToolSelector::sendDrawMode);
}
