// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/widthselectionbutton.h"

#include <QJsonArray>
#include <QString>

#include "src/drawing/drawtool.h"
#include "src/gui/peniconengine.h"

WidthSelectionButton::WidthSelectionButton(const QJsonArray &array,
                                           QWidget *parent)
    : ToolPropertyButton(parent)
{
  setToolTip(tr("select width of draw tool"));
  qreal width;
  for (const auto &item : array) {
    width = item.toDouble(-1.);
    if (width > 0.) {
      QIcon icon(new PenIconEngine(width, Qt::SolidLine));
      addItem(icon, QString::number(width, 'g', 2), width);
    }
  }
}

void WidthSelectionButton::setToolProperty(Tool *tool) const
{
  const qreal width = currentData().value<qreal>();
  if (width <= 0.) return;
  if (tool && tool->tool() & Tool::AnyDrawTool)
    static_cast<DrawTool *>(tool)->setWidth(width);
  emit sendToolProperties(tool_variant(width));
}

void WidthSelectionButton::toolChanged(const Tool *tool)
{
  if (tool && tool->tool() & Tool::AnyDrawTool) {
    const int idx = findData(static_cast<const DrawTool *>(tool)->width());
    if (idx >= 0) setCurrentIndex(idx);
  }
}
