// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef DRAGTOOL_H
#define DRAGTOOL_H

#include <QPointF>
#include <cmath>

#include "src/drawing/tool.h"

/** Experimental: DragTool for dragging view */
class DragTool : public Tool
{
  QPointF reference_point;
  static constexpr int skip_events = 3;
  int event_counter = skip_events - 1;

 public:
  DragTool(const int device = AnyDevice) noexcept : Tool(DragViewTool, device)
  {
  }
  DragTool(const DragTool &other) noexcept : Tool(DragViewTool, other.device())
  {
  }
  ~DragTool() {}
  const QPointF &reference() const noexcept { return reference_point; }
  void setReference(const QPointF &pos) noexcept { reference_point = pos; }
  QPointF dragTo(const QPointF target, const bool force) noexcept
  {
    if (force || ++event_counter >= skip_events)
      event_counter = 0;
    else
      return QPointF();
    return reference_point - target;
  }
};

#endif  // DRAGTOOL_H
