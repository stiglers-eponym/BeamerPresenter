// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef FLEXGRAPHICSLINEITEM_H
#define FLEXGRAPHICSLINEITEM_H

#include <QGraphicsLineItem>
#include <QLineF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

#include "src/config.h"
#include "src/enumerates.h"

class QWidget;
class QStyleOptionGraphicsItem;

/**
 * @brief QGraphicsLineItem with adaptable CompositionMode.
 *
 * Identical to QGraphicsLineItem, with the only difference that
 * it has a property composition_mode and adjusts QPainter::CompositionMode
 * before painting.
 */
class FlexGraphicsLineItem : public QGraphicsLineItem
{
  /// Composition mode used for this line.
  const QPainter::CompositionMode mode;

 public:
  /// Custom type of QGraphicsItem.
  enum { Type = UserType + FlexGraphicsLineItemType };

  /// Constructor.
  FlexGraphicsLineItem(
      const QLineF &line,
      QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver)
      : QGraphicsLineItem(line), mode(mode)
  {
  }

  /// Paint line to painter.
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                     QWidget *widget = nullptr) override
  {
    painter->setCompositionMode(mode);
    QGraphicsLineItem::paint(painter, option, widget);
  }

  /// @return custom QGraphicsItem type
  int type() const noexcept override { return Type; }
};

#endif  // FLEXGRAPHICSLINEITEM_H
