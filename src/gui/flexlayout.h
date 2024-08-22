// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef FLEXLAYOUT_H
#define FLEXLAYOUT_H

#include <QBoxLayout>
#include <QList>
#include <QObject>
#include <QRect>
#include <QSize>

#include "src/config.h"

class QWidget;
class QLayoutItem;

/**
 * @brief Box layout for fixed aspect.
 *
 * Layout class which tries to optimize combinations of vertical and
 * horizontal box layout containing widgets of fixed aspect ratio.
 */
class FlexLayout : public QLayout
{
  Q_OBJECT

  static constexpr int ref_width = 4096;

  /// Box layout direction (horizontal or vertical)
  QBoxLayout::Direction direction;
  /// Items managed by this layout (see QLayout documentation)
  QList<QLayoutItem *> items;

 public:
  /// Trivial constructor.
  FlexLayout(QBoxLayout::Direction direction) noexcept
      : QLayout(), direction(direction) {};

  /// Trivial constructor.
  FlexLayout(QBoxLayout::Direction direction, QWidget *parent) noexcept
      : QLayout(parent), direction(direction) {};

  /// Destructor: delete all items
  ~FlexLayout();

  /// Append item.
  void addItem(QLayoutItem *item) noexcept override { items.append(item); }

  /// Size hint of layout. Only the aspect ratio is relevant.
  QSize sizeHint() const override;

  /// Minimum size of layout, currently always zero.
  QSize minimumSize() const override { return {0, 0}; }

  /// Number of items in layout.
  int count() const noexcept override { return items.size(); }

  /// Get item, or nullptr if index is out of range.
  QLayoutItem *itemAt(const int index) const override
  {
    return items.value(index);
  }

  /// Get and remove item at given index. Return nullptr if index is out
  /// of range.
  QLayoutItem *takeAt(const int index) noexcept override
  {
    return items.size() > index && index >= 0 ? items.takeAt(index) : nullptr;
  }

  /// Update layout with new geometry.
  void setGeometry(const QRect &rect) override;
};

#endif  // FLEXLAYOUT_H
