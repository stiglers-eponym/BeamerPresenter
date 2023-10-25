// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/brushstylebutton.h"

#include <QVariant>

#include "src/drawing/drawtool.h"
#include "src/gui/brushiconengine.h"

BrushStyleButton::BrushStyleButton(QWidget *parent) : ToolPropertyButton(parent)
{
  for (auto it = brush_style_codes.keyBegin(); it != brush_style_codes.keyEnd();
       ++it) {
    QIcon icon(new BrushIconEngine(*it));
    addItem(icon, "", QVariant::fromValue(*it));
  }
}

void BrushStyleButton::setToolProperty(Tool *tool) const
{
  const Qt::BrushStyle style = currentData().value<Qt::BrushStyle>();
  if (tool && tool->tool() & Tool::AnyDrawTool) {
    auto draw_tool = static_cast<DrawTool *>(tool);
    if (draw_tool->brush().style() == Qt::NoBrush)
      draw_tool->brush().setColor(draw_tool->color());
    draw_tool->brush().setStyle(style);
  }
  emit sendToolProperties(tool_variant(style));
}

void BrushStyleButton::toolChanged(const Tool *tool)
{
  if (tool && tool->tool() & Tool::AnyDrawTool) {
    const int index = findData(QVariant::fromValue(
        static_cast<const DrawTool *>(tool)->brush().style()));
    if (index >= 0) setCurrentIndex(index);
  }
}
