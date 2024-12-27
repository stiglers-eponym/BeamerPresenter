// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/dragtool.h"

#include <QJsonObject>

void DragTool::setFlags(const QJsonObject &json) noexcept
{
  _flags.setFlag(TouchZoom, json.value("touch-zoom").toBool(true));
  const QString scroll = json.value("scroll").toString("zoom");
  _flags.setFlag(ScrollWheelZoom, scroll == "zoom");
  _flags.setFlag(ScrollWheelMove, scroll == "move");
  const QString dclick = json.value("double-click").toString("zoom");
  _flags.setFlag(DoubleClickZoom, dclick == "zoom");
}

QPointF DragTool::dragTo(const QPointF target, const bool force) noexcept
{
  if (force || ++event_counter >= skip_events)
    event_counter = 0;
  else
    return QPointF();
  if (reference_point.isNull()) reference_point = target;
  return reference_point - target;
}

QString DragTool::description() const noexcept
{
  QStringList zoom;
  if (_flags.testFlag(TouchZoom)) zoom.append(tr("touch pinch gesture"));
  if (_flags.testFlag(DoubleClickZoom)) zoom.append(tr("double click"));
  if (_flags.testFlag(ScrollWheelZoom)) zoom.append(tr("scroll wheel"));
  QString desc;
  if (zoom.isEmpty())
    desc = tr("drag/move view.");
  else
    desc = tr("drag/move/zoom view. Zoom using ") + zoom.join(", ") + ".";
  if (_flags.testFlag(ScrollWheelMove)) desc += tr(" Scroll to move up/down.");
  return desc;
}
