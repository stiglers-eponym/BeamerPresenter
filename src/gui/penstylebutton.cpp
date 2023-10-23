// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QVariant>
#include "src/gui/penstylebutton.h"
#include "src/drawing/drawtool.h"
#include "src/gui/peniconengine.h"

PenStyleButton::PenStyleButton(QWidget *parent) :
    ToolPropertyButton(parent)
{
    for (auto it = pen_style_codes.keyBegin(); it != pen_style_codes.keyEnd(); ++it)
    {
        QIcon icon(new PenIconEngine(2.5, *it));
        addItem(icon, "", QVariant::fromValue(*it));
    }
}

void PenStyleButton::setToolProperty(Tool *tool) const
{
    const Qt::PenStyle style = currentData().value<Qt::PenStyle>();
    if (tool && tool->tool() & Tool::AnyDrawTool)
        static_cast<DrawTool*>(tool)->rpen().setStyle(style);
    emit sendToolProperties(tool_variant(style));
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
