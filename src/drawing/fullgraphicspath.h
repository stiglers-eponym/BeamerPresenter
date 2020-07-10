#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QPainter>
#include <QPen>

struct PointPressure
{
    QPointF point;
    qreal pressure;
};

/// Variable width path.
class FullGraphicsPath : public AbstractGraphicsPath
{
    QVector<PointPressure> data;
    /// Pen for stroking path.
    /// pen.width will change with each drawn point.
    QPen pen;
    /// Bounding rect coordinates
    qreal top=0, bottom=0, left=0, right=0;

public:
    enum { Type = UserType + 2 };
    FullGraphicsPath();
    int type() const override {return Type;}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    /// Add a point to data and update bounding rect.
    void addPoint(QPointF const& point, qreal const pressure);
    QRectF boundingRect() const override {return QRectF(left, top, right-left, bottom-top);}
};

#endif // FULLGRAPHICSPATH_H
