#ifndef BASICGRAPHICSPATH_H
#define BASICGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"
#include <QPen>

/// Fixed width path.
class BasicGraphicsPath : public AbstractGraphicsPath, QGraphicsPathItem
{
public:
    enum { Type = UserType + 1 };
    BasicGraphicsPath();
    int type() const override {return Type;}
};

#endif // BASICGRAPHICSPATH_H
