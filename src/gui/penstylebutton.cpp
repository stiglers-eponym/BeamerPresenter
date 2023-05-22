// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QVariant>
#include "src/gui/penstylebutton.h"
#include "src/drawing/drawtool.h"
#include "src/gui/peniconengine.h"

PenStyleButton::PenStyleButton(QWidget *parent) :
    ToolPropertyButton(parent)
{
    for (const Qt::PenStyle style : {Qt::SolidLine,Qt::DashLine,Qt::DotLine,Qt::DashDotLine,Qt::DashDotDotLine})
    {
        QIcon icon(new PenIconEngine(2.5, style));
        addItem(icon, "", QVariant::fromValue(style));
    }
}

void PenStyleButton::setToolProperty(Tool *tool) const
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
        static_cast<DrawTool*>(tool)->rpen().setStyle(currentData(Qt::UserRole).value<Qt::PenStyle>());
}

void PenStyleButton::toolChanged(const Tool *tool)
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
    {
        const int index = findData(QVariant::fromValue(static_cast<const DrawTool*>(tool)->pen().style()));
        if (index >= 0)
            setCurrentIndex(index);
    }
}
