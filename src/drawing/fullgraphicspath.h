#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QGraphicsScene>
#include <QPainter>


/// Coordinate (point) and pen pressure representing node of FullGraphicsPath.
struct PointPressure
{
    /// position in points.
    QPointF point;

    /// stroke width in points.
    float pressure;
};


/// Variable width path.
/// TODO: flexible stroke width (relative to pressure) and color
class FullGraphicsPath : public AbstractGraphicsPath
{
    /// Vector of nodes (coordinates and pressure).
    QVector<PointPressure> data;

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + 2 };

    /// Construct path with given initial node and default pen.
    FullGraphicsPath(const QPointF &pos, const float pressure);

    /// Construct subpath of other FullGraphicsPath, including nodes first to
    /// last-1 of other.
    FullGraphicsPath(const FullGraphicsPath *const other, int first, int last);

    /// return custom type of QGraphicsItem.
    int type() const override
    {return Type;}

    /// Number of nodes of the path.
    int size() const override
    {return data.size();}

    /// Position of last node in the path.
    /// Returns QPointF() if path is empty.
    const QPointF lastPoint() override
    {return data.isEmpty() ? QPointF() : data.last().point;}

    /// Paint this on given painter. Options and widget are currently
    /// discarded.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point, const float pressure);

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;
};

#endif // FULLGRAPHICSPATH_H
