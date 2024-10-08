// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PENICONENGINE_H
#define PENICONENGINE_H

#include <QColor>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QRect>

#include "src/config.h"

/// Generate icon showing a single line with given width and style.
class PenIconEngine : public QIconEngine
{
  /// Pen used to draw on the icon.
  QPen pen;

  static constexpr qreal pixel_pt_conversion = 1.5;

 public:
  /// Constructor: initialize pen.
  PenIconEngine(qreal width, Qt::PenStyle style)
      : pen(Qt::black, pixel_pt_conversion * width, style)
  {
  }

  /// Trivial destructor.
  ~PenIconEngine() {}

  /// Draw horizontal line with given pen.
  void paint(QPainter *painter, const QRect &rect, QIcon::Mode,
             QIcon::State) override
  {
    painter->setPen(pen);
    const qreal y = (rect.top() + rect.bottom()) / 2;
    painter->drawLine(rect.left() + 1, y, rect.right() - 1, y);
  }

  /// Clone this item.
  QIconEngine *clone() const override
  {
    return new PenIconEngine(pen.widthF(), pen.style());
  }

  /// Draw transparent pixmap showing horizontal line with given pen.
  QPixmap pixmap(const QSize &size, QIcon::Mode, QIcon::State) override
  {
    QPixmap pixmap(size);
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter;
    painter.begin(&pixmap);
    painter.setPen(pen);
    painter.drawLine(1, size.height() / 2, size.width() - 1, size.height() / 2);
    painter.end();
    return pixmap;
  }
};

#endif  // PENICONENGINE_H
