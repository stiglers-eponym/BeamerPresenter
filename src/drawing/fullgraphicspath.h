#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QGraphicsScene>
#include <QPainter>

struct PointPressure
{
    QPointF point;
    float pressure;
};

/// Variable width path.
/// TODO: flexible stroke width (relative to pressure) and color
class FullGraphicsPath : public AbstractGraphicsPath
{
    QVector<PointPressure> data;

public:
    enum { Type = UserType + 2 };
    FullGraphicsPath(const QPointF &pos, const float pressure);
    FullGraphicsPath(const FullGraphicsPath *const other, int first, int last);
    int type() const override {return Type;}
    int size() const override {return data.size();}
    /// Position of last point in the path.
    const QPointF lastPoint() override
    {return data.isEmpty() ? QPointF() : data.last().point;}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point, const float pressure);

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;
};

#endif // FULLGRAPHICSPATH_H
