#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QGraphicsScene>
#include <QPainter>
#include <QPen>

struct PointPressure
{
    QPointF point;
    float pressure;
};

/// Variable width path.
class FullGraphicsPath : public AbstractGraphicsPath
{
    QVector<PointPressure> data;
    /// Pen for stroking path.
    /// pen.width will change with each drawn point.
    QPen pen {QBrush(Qt::red), 1., Qt::SolidLine, Qt::RoundCap};
    /// Bounding rect coordinates
    qreal top, bottom, left, right;

public:
    enum { Type = UserType + 2 };
    FullGraphicsPath(const QPointF &pos, const float pressure);
    FullGraphicsPath(const FullGraphicsPath *other, int first, int last);
    int type() const override {return Type;}
    int size() const override {return data.size();}
    const QPointF lastPoint() override {return data.isEmpty() ? QPointF() : data.last().point;}
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point, const float pressure);
    QRectF boundingRect() const override {return QRectF(left, top, right-left, bottom-top);}
    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;
};

#endif // FULLGRAPHICSPATH_H
