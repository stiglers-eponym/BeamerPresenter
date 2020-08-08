#ifndef BASICGRAPHICSPATH_H
#define BASICGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QPainter>
#include <QGraphicsScene>

/// Fixed width path.
class BasicGraphicsPath : public AbstractGraphicsPath
{
    QVector<QPointF> data;

public:
    enum { Type = UserType + 1 };
    BasicGraphicsPath(const QPointF &pos);
    BasicGraphicsPath(const BasicGraphicsPath *const other, int first, int last);
    int type() const override {return Type;}
    int size() const override {return data.size();}
    /// Position of last point in the path.
    const QPointF lastPoint() override
    {return data.isEmpty() ? QPointF() : data.last();}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point);

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;
};

#endif // BASICGRAPHICSPATH_H
