#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QGraphicsItem>

class AbstractGraphicsPath : public QGraphicsItem
{

public:
    AbstractGraphicsPath();
    virtual int size() const = 0;
    virtual const QPointF lastPoint() = 0;
};

#endif // ABSTRACTGRAPHICSPATH_H
