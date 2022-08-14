// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/shapeselectionbutton.h"
#include "src/drawing/drawtool.h"
#include "src/preferences.h"

ShapeSelectionButton::ShapeSelectionButton(QWidget *parent) :
    ToolPropertyButton(parent)
{
    QIcon icon(preferences()->icon_path + "/tools/freehand.svg");
    if (icon.isNull())
        addItem("freehand");
    else
        addItem(icon, "");
    icon = QIcon(preferences()->icon_path + "/tools/rectangle.svg");
    if (icon.isNull())
        addItem("rectangle");
    else
        addItem(icon, "");
    icon = QIcon(preferences()->icon_path + "/tools/ellipse.svg");
    if (icon.isNull())
        addItem("ellipse");
    else
        addItem(icon, "");
    icon = QIcon(preferences()->icon_path + "/tools/arrow.svg");
    if (icon.isNull())
        addItem("arrow");
    else
        addItem(icon, "");
    icon = QIcon(preferences()->icon_path + "/tools/line.svg");
    if (icon.isNull())
        addItem("line");
    else
        addItem(icon, "");
    icon = QIcon(preferences()->icon_path + "/tools/recognize.svg");
    if (icon.isNull())
        addItem("recognize");
    else
        addItem(icon, "");
}

void ShapeSelectionButton::setToolProperty(Tool *tool) const
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
        static_cast<DrawTool*>(tool)->setShape(DrawTool::Shape(currentIndex()));
}

void ShapeSelectionButton::updateTool()
{
    const Tool *tool = preferences()->currentTool(device);
    if (tool && tool->tool() & Tool::AnyDrawTool)
        setCurrentIndex(int(static_cast<const DrawTool*>(tool)->shape()));
}

void ShapeSelectionButton::toolChanged(Tool *tool)
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
        setCurrentIndex(int(static_cast<const DrawTool*>(tool)->shape()));
}
