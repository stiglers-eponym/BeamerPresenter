// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/pointingtool.h"

#include <QRadialGradient>
#include <QtConfig>

constexpr qreal POINTER_INNER_RADIUS = 0.1;
constexpr qreal POINTER_OUTER_RADIUS = 0.9;
constexpr int POINTER_OUTER_ALPHA = 12;

void PointingTool::initPointerBrush() noexcept
{
  QRadialGradient grad(.5, .5, .5);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
  grad.setCoordinateMode(QGradient::ObjectMode);
#else
  grad.setCoordinateMode(QGradient::ObjectBoundingMode);
#endif
  const QColor color = _brush.color();
  grad.setColorAt(POINTER_INNER_RADIUS, color);
  QColor semitransparent = color;
  semitransparent.setAlpha(POINTER_OUTER_ALPHA);
  grad.setColorAt(POINTER_OUTER_RADIUS, semitransparent);
  semitransparent.setAlpha(0);
  grad.setColorAt(1, semitransparent);
  _brush = QBrush(grad);
  _brush.setColor(color);
}

void PointingTool::setColor(const QColor &color) noexcept
{
  _brush.setColor(color);
  if (_tool == Pointer) initPointerBrush();
}
