// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef BASICGRAPHICSPATH_H
#define BASICGRAPHICSPATH_H

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

#include "src/config.h"
#include "src/drawing/abstractgraphicspath.h"
#include "src/drawing/drawtool.h"
#include "src/enumerates.h"

class QPainter;
class QWidget;
class QStyleOptionGraphicsItem;

/**
 * @brief Fixed width graphics path.
 *
 * QGraphicsItem representing a path with fixed stroke width.
 * @see AbstractGraphicsPath
 */
class BasicGraphicsPath : public AbstractGraphicsPath
{
 protected:
  static constexpr qreal width_tolerance = 0.01;

 public:
  /// Custom type of QGraphicsItem.
  enum { Type = UserType + BasicGraphicsPathType };

  /// Construct path starting at given position.
  /// @param tool draw tool for stroking path
  /// @param pos first node (coordinate) in this path
  BasicGraphicsPath(const DrawTool &tool, const QPointF &pos) noexcept;

  /// Construct path from coordinate vector and given bounding rect.
  /// @param tool draw tool for stroking path
  /// @param coordinates vector
  /// @param bounding_rect bounding rectangle
  BasicGraphicsPath(const DrawTool &tool, const QVector<QPointF> &coordinates,
                    const QRectF &bounding_rect = QRectF()) noexcept;

  /// Construct path from coordinate string.
  /// @param tool draw tool for stroking path
  /// @param coordinates string representing nodes as space separated numbers
  ///        in the form x1 y1 x2 y2 x3 y3 ...
  BasicGraphicsPath(const DrawTool &tool,
                    const QString &coordinate_string) noexcept;

  /// Construct subpath of other BasicGraphicsPath, including nodes first to
  /// last-1 of other.
  /// @param other other graphics path from which a subpath should be created
  /// @param first index of first node contained in the subpath.
  /// @param last index of first node after the subpath.
  BasicGraphicsPath(const AbstractGraphicsPath *const other, int first,
                    int last);

  /// Copy this.
  AbstractGraphicsPath *copy() const override;

  /// @return custom type of QGraphicsItem.
  int type() const noexcept override { return Type; }

  /// Paint this on given painter.
  /// @param painter paint to this painter.
  /// @param option currently ignored.
  /// @param widget currently ignored.
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr) override;

  /// Add a point to coordinates and update bounding rect.
  /// @param point new node
  void addPoint(const QPointF &point);

  QList<AbstractGraphicsPath *> splitErase(const QPointF &scene_pos,
                                           const qreal size) const override;

  void changeTool(const DrawTool &newtool) noexcept override;

  /// Write stroke width to string for saving.
  /// @return string representing width of tool
  const QString stringWidth() const noexcept override
  {
    return QString::number(_tool.width());
  }
};

#endif  // BASICGRAPHICSPATH_H
