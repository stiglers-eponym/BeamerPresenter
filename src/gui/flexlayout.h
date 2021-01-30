#ifndef FLEXLAYOUT_H
#define FLEXLAYOUT_H

#include <QDebug>
#include <QLayout>
#include <QWidget>
#include "src/preferences.h"

/**
 * @brief FlexLayout class: Box layout for fixed aspect
 * @abstract
 * Layout class which tries to optimize combinations of vertical and
 * horizontal box layout containing widgets of fixed aspect ratio.
 *
 * TODO: The layout is broken when testing in Xubuntu 20.04
 */
class FlexLayout : public QLayout
{
    Q_OBJECT

    QBoxLayout::Direction direction;
    QVector<QLayoutItem*> items;

public:
    /// Trivial constructor.
    FlexLayout(QBoxLayout::Direction direction) noexcept : QLayout(), direction(direction) {};

    /// Trivial constructor.
    FlexLayout(QBoxLayout::Direction direction, QWidget *parent) noexcept : QLayout(parent), direction(direction) {};

    /// Destructor: delete all items
    ~FlexLayout();

    /// Append item.
    void addItem(QLayoutItem *item) noexcept override
    {items.append(item);}

    /// Size hint of layout. Only the aspect ratio is relevant.
    QSize sizeHint() const override;

    /// Minimum size of layout, currently always zero.
    QSize minimumSize() const override
    {return {0,0};}

    /// Number of items in layout.
    int count() const noexcept override
    {return items.size();}

    /// Get item, or NULL if index is out of range.
    QLayoutItem *itemAt(const int index) const override
    {return items.value(index);}

    /// Get and remove item at given index. Return NULL if index is out
    /// of range.
    QLayoutItem *takeAt(const int index) noexcept override
    {return items.size() > index && index >= 0 ? items.takeAt(index) : NULL;}

    void setGeometry(const QRect &rect) override;
};

#endif // FLEXLAYOUT_H
