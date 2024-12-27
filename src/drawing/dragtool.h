// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef DRAGTOOL_H
#define DRAGTOOL_H

#include <QFlags>
#include <QPointF>

#include "src/drawing/tool.h"

class QJsonObject;

/** Experimental: DragTool for dragging view */
class DragTool : public Tool
{
  Q_DECLARE_TR_FUNCTIONS(DragTool)

 public:
  enum DragToolFlag {
    TouchZoom = 1 << 0,
    ScrollWheelZoom = 1 << 1,
    ScrollWheelMove = 1 << 2,
    DoubleClickZoom = 1 << 3,
  };
  Q_DECLARE_FLAGS(DragToolFlags, DragToolFlag);
  Q_FLAG(DragToolFlags);

 private:
  DragToolFlags _flags = {TouchZoom ^ ScrollWheelZoom ^ DoubleClickZoom};
  QPointF reference_point;
  static constexpr int skip_events = 3;
  int event_counter = skip_events - 1;

 public:
  DragTool(const int device = AnyDevice) noexcept : Tool(DragViewTool, device)
  {
  }

  DragTool(const DragTool &other) noexcept
      : Tool(DragViewTool, other.device()),
        _flags(other._flags),
        reference_point(other.reference_point)
  {
  }

  ~DragTool() {}

  /// Read flags from JSON object.
  void setFlags(const QJsonObject &json) noexcept;

  const DragToolFlags &flags() const noexcept { return _flags; }

  const QPointF &reference() const noexcept { return reference_point; }

  void setReference(const QPointF &pos) noexcept { reference_point = pos; }

  void clear() noexcept { reference_point = QPointF(); }

  virtual QString description() const noexcept;

  /** Set reference_point to target and return difference (reference_point -
   * target). When calling this function DragTool::skip_events times, it will
   * only take effect once and skip the other cases, except if force is set to
   * true. */
  QPointF dragTo(const QPointF target, const bool force) noexcept;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DragTool::DragToolFlags);

#endif  // DRAGTOOL_H
