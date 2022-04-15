#ifndef SELECTIONRECTITEM_H
#define SELECTIONRECTITEM_H

#include <QGraphicsItem>

class SelectionRectItem : public QGraphicsItem
{
    QRectF _rect;
public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 10};

    /// Trivial constructor.
    SelectionRectItem(QGraphicsItem *parent = NULL) : QGraphicsItem(parent)
    {setZValue(1e2);}

    /// @return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Paint this on given painter.
    /// @param painter paint to this painter.
    /// @param option currently ignored.
    /// @param widget currently ignored.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    void setRect(const QRectF &rect)
    {setPos(0,0); _rect = rect;}

    QPolygonF sceneRect() const noexcept
    {return mapToScene(_rect);}

    QPointF sceneRotationHandle() const noexcept
    {return mapToScene(_rect.left()+_rect.width()/2, _rect.top()-10);}

    virtual QRectF boundingRect() const noexcept override
    {return _rect.marginsAdded(QMarginsF(4,14,4,4));}
};

#endif // SELECTIONRECTITEM_H
