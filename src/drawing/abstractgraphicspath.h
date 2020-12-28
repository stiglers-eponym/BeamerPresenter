#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QGraphicsItem>
#include <QPen>

/// Coordinates and pen for drawing a path.
/// Coordinates are given as positions in the PDF page, measured in points
/// as floating point values. These are the same units as used in SlideScene.
class AbstractGraphicsPath : public QGraphicsItem
{
protected:
    /// Pen for stroking path.
    /// in FullGraphicsPath pen.width will change with each drawn point.
    QPen pen {QBrush(Qt::red), 1., Qt::SolidLine, Qt::RoundCap};
    /// Bounding rect coordinates
    qreal top, bottom, left, right;

public:
    /// Trivial constructor.
    AbstractGraphicsPath() {}
    /// Constructor initializing pen.
    AbstractGraphicsPath(const QPen pen) : pen(pen) {}
    /// Bounding rectangle of the drawing (including stroke width).
    QRectF boundingRect() const override {return QRectF(left, top, right-left, bottom-top);}
    /// Number of nodes of the path.
    virtual int size() const = 0;
    /// Coordinate of the last node in the path.
    virtual const QPointF lastPoint() = 0;

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    virtual QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const = 0;
};

#endif // ABSTRACTGRAPHICSPATH_H
