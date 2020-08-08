#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QGraphicsItem>
#include <QPen>

class AbstractGraphicsPath : public QGraphicsItem
{
protected:
    /// Pen for stroking path.
    /// pen.width will change with each drawn point.
    QPen pen {QBrush(Qt::red), 1., Qt::SolidLine, Qt::RoundCap};
    /// Bounding rect coordinates
    qreal top, bottom, left, right;

public:
    AbstractGraphicsPath() {}
    AbstractGraphicsPath(QPen pen) : pen(pen) {}
    QRectF boundingRect() const override {return QRectF(left, top, right-left, bottom-top);}
    virtual int size() const = 0;
    virtual const QPointF lastPoint() = 0;

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    virtual QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const = 0;
};

#endif // ABSTRACTGRAPHICSPATH_H
