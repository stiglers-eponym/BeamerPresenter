// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/shapeselectionbutton.h"

#include <QIcon>

#include "src/drawing/drawtool.h"
#include "src/preferences.h"

ShapeSelectionButton::ShapeSelectionButton(QWidget *parent)
    : ToolPropertyButton(parent)
{
  static const QMap<DrawTool::Shape, const char *> shape_to_name = {
      {DrawTool::Freehand, "freehand"}, {DrawTool::Rect, "rectangle"},
      {DrawTool::Ellipse, "ellipse"},   {DrawTool::Arrow, "arrow"},
      {DrawTool::Line, "line"},         {DrawTool::Recognize, "recognize"},
  };

  setToolTip(tr("select shape to be drawn"));
  const QString &icon_path = preferences()->icon_path + "/tools/";
  for (auto it = shape_to_name.cbegin(); it != shape_to_name.cend(); ++it) {
    QIcon icon(icon_path + *it + ".svg");
    if (icon.isNull())
      addItem(*it, it.key());
    else
      addItem(icon, "", it.key());
  }
}

void ShapeSelectionButton::setToolProperty(std::shared_ptr<Tool> tool) const
{
  if (tool && tool->tool() & Tool::AnyDrawTool)
    std::static_pointer_cast<DrawTool>(tool)->setShape(
        currentData().value<DrawTool::Shape>());
}

void ShapeSelectionButton::toolChanged(std::shared_ptr<const Tool> tool)
{
  if (tool && tool->tool() & Tool::AnyDrawTool) {
    const int index =
        findData(std::static_pointer_cast<const DrawTool>(tool)->shape());
    if (index >= 0) setCurrentIndex(index);
  }
}
