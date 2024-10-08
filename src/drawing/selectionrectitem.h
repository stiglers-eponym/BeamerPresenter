// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SELECTIONRECTITEM_H
#define SELECTIONRECTITEM_H

#include <QGraphicsItem>
#include <QMarginsF>
#include <QPolygonF>
#include <QRectF>

#include "src/config.h"
#include "src/enumerates.h"
#include "src/preferences.h"

class QWidget;
class QPainter;
class QStyleOptionGraphicsItem;

/**
 * @brief Visualization of rectangle selection.
 */
class SelectionRectItem : public QGraphicsItem
{
  static constexpr qreal selection_handle_separation = 1.5;
  static constexpr int selection_rect_line_alpha = 196;

  QRectF _rect;

 public:
  /// QGraphicsItem type for this subclass
  enum { Type = UserType + SelectionRectItemType };

  /// Trivial constructor.
  SelectionRectItem(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent)
  {
    setZValue(1e2);
  }

  /// @return custom type of QGraphicsItem.
  int type() const noexcept override { return Type; }

  /// Paint this on given painter.
  /// @param painter paint to this painter.
  /// @param option currently ignored.
  /// @param widget currently ignored.
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget = nullptr) override;

  /// reset transform and set rect (in scene coordinates)
  void setRect(const QRectF &rect) noexcept;

  /// return (possibly rotated) rectangle in scene coordinates
  QPolygonF sceneRect() const noexcept { return mapToScene(_rect); }

  /// return rectangle center in scene coordinates
  QPointF sceneCenter() const noexcept { return mapToScene(_rect.center()); }

  /// return center of rotation handle in scene coordinates
  QPointF sceneRotationHandle() const noexcept
  {
    return mapToScene(
        _rect.left() + _rect.width() / 2,
        _rect.top() - 1.5 * preferences()->selection_rect_handle_size);
  }

  // Return center of delete handle in scene coordinates
  QPointF sceneDeleteHandle() const noexcept
  {
    const qreal handle_size = preferences()->selection_rect_handle_size;
    return mapToScene(_rect.left() + _rect.width() / 2 + 2 * handle_size,
                      _rect.top() - 1.5 * handle_size);
  }

  /// return a polygon containing the corners of this rectangle in scene
  /// coordinates
  QPolygonF scaleHandles() const noexcept;

  virtual QRectF boundingRect() const noexcept override
  {
    const qreal half_size = preferences()->selection_rect_handle_size / 2;
    const qreal stroke_width = preferences()->selection_rect_pen.widthF() / 2;
    return _rect.marginsAdded(
        QMarginsF(half_size + stroke_width, 4 * half_size + stroke_width,
                  half_size + stroke_width, half_size + stroke_width));
  }

  /// return whether point in scene coordinates is contained in _rect
  bool containsPoint(const QPointF scene_point) const noexcept
  {
    return _rect.contains(mapFromScene(scene_point));
  }
};

#endif  // SELECTIONRECTITEM_H
