// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef DRAGTOOL_H
#define DRAGTOOL_H

#include <QFlags>
#include <QPointF>

#include "src/drawing/tool.h"

class QJsonObject;

/** DragTool for moving and zooming view */
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
  /// Flags defining how input is handled.
  DragToolFlags _flags = {TouchZoom ^ ScrollWheelZoom ^ DoubleClickZoom};
  /// When dragging the view, only one out of skip_events events will be
  /// handled and the other events will be ignored. This done is because
  /// moving the view will cause another event that cannot simply be
  /// distingished from "real" events caused by the user.
  static constexpr qint8 skip_events = 3;
  /// counter of events, used to determine whether the next event should
  /// be handled or ignored.
  qint8 event_counter = skip_events - 1;
  /// When dragging the view using a pointing tool, reference_point is
  /// the last position of the pointing tool.
  QPointF reference_point;

 public:
  DragTool(const Tool::InputDevices device = AnyDevice) noexcept
      : Tool(DragViewTool, device)
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
