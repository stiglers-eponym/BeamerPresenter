// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef GRAPHICSPICTUREITEM_H
#define GRAPHICSPICTUREITEM_H

#include <QPicture>
#include <QPainter>
#include <QGraphicsItem>
#include "src/config.h"
#include "src/enumerates.h"

class QStyleOptionGraphicsItem;

/**
 * @brief QGraphicsItem showing a QPicture
 */
class GraphicsPictureItem : public QGraphicsItem
{
    /// Picture to be drawn on this item.
    QPicture _picture;

public:
    /// QGraphicsItem type for this subclass
    enum { Type = UserType + GraphicsPictureItemType };

    /// Constructor: set default flags.
    GraphicsPictureItem(const QPicture &picture, QGraphicsItem *parent = nullptr) :
        QGraphicsItem(parent), _picture(picture)
    {setFlags(ItemIsMovable | ItemIsSelectable);}

    /// Trivial destructor
    ~GraphicsPictureItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    QPicture &picture() noexcept
    {return _picture;}

    const QPicture &picture() const noexcept
    {return _picture;}

    /// Paint picture to the painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget = nullptr) override
    {_picture.play(painter);}

    /// Bounding rectangle of the picture
    virtual QRectF boundingRect() const noexcept override
    {return _picture.boundingRect();}

    /// Clone this item.
    GraphicsPictureItem *copy() const noexcept
    {
        auto item = new GraphicsPictureItem(_picture);
        item->setZValue(zValue());
        return item;
    }

    /// Check if picture is null.
    bool empty() const noexcept
    {return _picture.isNull();}
};

#endif // GRAPHICSPICTUREITEM_H
