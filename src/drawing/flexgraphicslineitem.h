#ifndef FLEXGRAPHICSLINEITEM_H
#define FLEXGRAPHICSLINEITEM_H

#include <QGraphicsLineItem>
#include <QPainter>

/**
 * @class FlexGraphicsLineItem
 * @brief QGraphicsLineItem with adaptable CompositionMode.
 * @abstract Identical to QGraphicsLineItem, with the only difference that
 * it has a property composition_mode and adjusts QPainter::CompositionMode
 * before painting.
 */
class FlexGraphicsLineItem : public QGraphicsLineItem
{
    QPainter::CompositionMode mode;

public:
    enum {Type = UserType + 3};
    FlexGraphicsLineItem(const QLineF& line, QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver) :
        QGraphicsLineItem(line), mode(mode) {}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override
    {painter->setCompositionMode(mode); QGraphicsLineItem::paint(painter, option, widget);}
};

#endif // FLEXGRAPHICSLINEITEM_H
