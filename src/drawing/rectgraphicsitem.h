// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef RECTGRAPHICSITEM_H
#define RECTGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include <QPointF>

#include "src/config.h"
#include "src/drawing/drawtool.h"
#include "src/enumerates.h"

class QWidget;
class QPainter;
class QStyleOptionGraphicsItem;
class BasicGraphicsPath;

/**
 * @brief RectGraphicsItem: QGraphicsRectItem adjusted for interactive drawing
 *
 * This class makes sure that the rect of the underlying QGraphicsRectItem is
 * always valid while moving one of the corners.
 */
class RectGraphicsItem : public QGraphicsRectItem
{
  enum {
    OriginRight = 0x1,
    OriginBottom = 0x2,
    TopLeft = 0x0,
    TopRight = OriginRight,
    BottomLeft = OriginBottom,
    BottomRight = OriginRight | OriginBottom,
  };
  /// DrawTool for this path.
  const DrawTool tool;
  /// Defines which corner of the rectangle is kept fixed.
  quint8 origin = TopLeft;

 public:
  /// QGraphicsItem type for this subclass
  enum { Type = UserType + RectGraphicsItemType };

  /// Constructor for initializing QGraphicsRectItem
  /// @param pos origin of the rectangle. This coordinate is always fixed.
  RectGraphicsItem(const DrawTool &tool, const QPointF &pos,
                   QGraphicsItem *parent = nullptr);

  /// Trivial destructor.
  ~RectGraphicsItem() {}

  /// @return custom QGraphicsItem type.
  int type() const noexcept override { return Type; }

  /// Change the flexible coordinate of the rectangle.
  /// Make sure that the underlying rect is always valid.
  void setSecondPoint(const QPointF &pos);

  /// Convert to a BasicGraphicsPath for simpler erasing.
  BasicGraphicsPath *toPath() const;

  /// Paint line to painter.
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget = nullptr) override
  {
    painter->setCompositionMode(tool.compositionMode());
    QGraphicsRectItem::paint(painter, option, widget);
  }
};

#endif  // RECTGRAPHICSITEM_H
