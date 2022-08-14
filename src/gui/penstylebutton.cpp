// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/penstylebutton.h"
#include "src/drawing/drawtool.h"

PenStyleButton::PenStyleButton(QWidget *parent) :
    ToolPropertyButton(parent)
{
    addItem("—", QVariant::fromValue(Qt::PenStyle::SolidLine));
    addItem("- -", QVariant::fromValue(Qt::PenStyle::DashLine));
    addItem(" : ", QVariant::fromValue(Qt::PenStyle::DotLine));
    addItem("- ·", QVariant::fromValue(Qt::PenStyle::DashDotLine));
    addItem("- · ·", QVariant::fromValue(Qt::PenStyle::DashDotDotLine));
}

void PenStyleButton::setToolProperty(Tool *tool) const
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
        static_cast<DrawTool*>(tool)->rpen().setStyle(currentData(Qt::UserRole).value<Qt::PenStyle>());
}

void PenStyleButton::toolChanged(Tool *tool)
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
    {
        const int index = findData(QVariant::fromValue(static_cast<const DrawTool*>(tool)->pen().style()));
        if (index >= 0)
            setCurrentIndex(index);
    }
}
#include "penstylebutton.h"
