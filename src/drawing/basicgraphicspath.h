#ifndef BASICGRAPHICSPATH_H
#define BASICGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QPen>
#include <QGraphicsScene>

/// Fixed width path.
class BasicGraphicsPath : public AbstractGraphicsPath, QGraphicsPathItem
{
public:
    enum { Type = UserType + 1 };
    BasicGraphicsPath();
    int type() const override {return Type;}
    int size() const override {return 0;}
    const QPointF lastPoint() override {return QPointF();}
};

#endif // BASICGRAPHICSPATH_H
