#ifndef LINEGRAPHICSITEM_H
#define LINEGRAPHICSITEM_H

#include <QGraphicsLineItem>
#include "src/drawing/basicgraphicspath.h"

/**
 * @brief LineGraphicsItem: QGraphicsLineItem with a tool that can be converted to BasicGraphicsPath
 */
class LineGraphicsItem : public QGraphicsLineItem
{
    /// DrawTool for this path.
    const DrawTool tool;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 8};

    /// Constructor for initializing QGraphicsLineItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    LineGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = NULL) :
        QGraphicsLineItem(QLineF(pos, pos), parent),
        tool(tool)
    {
        setPen(tool.pen());
    }

    /// Trivial destructor.
    ~LineGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the line.
    void setSecondPoint(const QPointF &pos)
    {
        QLineF newline = line();
        newline.setP2(pos);
        setLine(newline);
    }

    /// Convert to a BasicGraphicsPath for simpler erasing.
    BasicGraphicsPath *toPath() const
    {
        if (line().p1() == line().p2())
            return NULL;
        const int segments = line().length() / 10 + 2;
        const qreal x = line().p1().x(),
                    y = line().p1().y(),
                    dx = line().dx()/segments,
                    dy = line().dy()/segments;
        QVector<QPointF> coordinates(segments+1);
        for (int i=0; i<=segments; ++i)
            coordinates[i] = {x+i*dx, y+i*dy};
        BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, boundingRect());
        return path;
    }

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override
    {
        painter->setCompositionMode(tool.compositionMode());
        QGraphicsLineItem::paint(painter, option, widget);
    }
};

#endif // LINEGRAPHICSITEM_H
