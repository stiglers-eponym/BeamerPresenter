#ifndef ARROWGRAPHICSITEM_H
#define ARROWGRAPHICSITEM_H

#include <cmath>
#include <QGraphicsPathItem>
#include "src/drawing/basicgraphicspath.h"

/**
 * Given the start and end point of an arrow, calculate the two other points
 * needed for an arrow.
 * @param start start point of the arrow
 * @param end end point of the arrow
 * @param p1 will be overwritten by first extra point for the arrow
 * @param p2 will be overwritten by second extra point for the arrow
 */
void calcArrowPoints(const QPointF &start, const QPointF &end, QPointF &p1, QPointF &p2)
{
    const qreal length = QLineF(start, end).length();
    const qreal scale1 = 8. / std::max(40., length) + 32. / std::max(320., length);
    const qreal scale2 = 1.2*scale1;
    p1 = {scale2*start.x() + (1-scale2)*end.x() - scale1*(end.y() - start.y()), scale2*start.y() + (1-scale2)*end.y() + scale1*(end.x() - start.x())};
    p2 = {scale2*start.x() + (1-scale2)*end.x() + scale1*(end.y() - start.y()), scale2*start.y() + (1-scale2)*end.y() - scale1*(end.x() - start.x())};
}

/**
 * @brief ArrowGraphicsItem: Path shaped like an arrow
 *
 * Given a fixed start point and a flexible (movable) end point, this class
 * draws the arrow and can export the path as BasicGraphicsPath.
 */
class ArrowGraphicsItem : public QGraphicsPathItem
{
    /// DrawTool for this path.
    const DrawTool tool;
    /// Origin of the arrow
    const QPointF origin;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 9};

    /// Constructor for initializing QGraphicsLineItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    ArrowGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = NULL) :
        QGraphicsPathItem(parent),
        origin(pos),
        tool(tool)
    {
        setPen(tool.pen());
    }

    /// Trivial destructor.
    ~ArrowGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the line.
    void setSecondPoint(const QPointF &pos)
    {
        QPainterPath newpath(origin);
        newpath.lineTo(pos);
        QPointF p1, p2;
        calcArrowPoints(origin, pos, p1, p2);
        newpath.lineTo(p1);
        newpath.moveTo(p2);
        newpath.lineTo(pos);
        setPath(newpath);
    }

    /// Convert to a BasicGraphicsPath for simpler erasing.
    BasicGraphicsPath *toPath() const
    {
        const QPointF end = path().currentPosition();
        const qreal length = QLineF(origin, end).length();
        const int main_segments = length / 10 + 2,
                aux_segments = length / 40 + 2;
        QPointF p1, p2;
        calcArrowPoints(origin, end, p1, p2);
        qreal x = origin.x(),
              y = origin.y(),
              dx = (end.x() - origin.x())/main_segments,
              dy = (end.y() - origin.y())/main_segments;
        QVector<QPointF> coordinates(main_segments+2*aux_segments+2);
        for (int i=0; i<=main_segments; ++i)
            coordinates[i] = {x+i*dx, y+i*dy};
        coordinates[main_segments] = end;
        x = p1.x();
        y = p1.y();
        dx = (end.x() - p1.x())/aux_segments;
        dy = (end.y() - p1.y())/aux_segments;
        int index_shift = main_segments + 1;
        for (int i=0; i<aux_segments; ++i)
            coordinates[index_shift + i] = {x+i*dx, y+i*dy};
        x = end.x();
        y = end.y();
        dx = (p2.x() - end.x())/aux_segments;
        dy = (p2.y() - end.y())/aux_segments;
        index_shift += aux_segments;
        for (int i=0; i<=aux_segments; ++i)
            coordinates[index_shift + i] = {x+i*dx, y+i*dy};
        BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates, boundingRect());
        return path;
    }

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override
    {
        painter->setCompositionMode(tool.compositionMode());
        QGraphicsPathItem::paint(painter, option, widget);
    }
};

#endif // ARROWGRAPHICSITEM_H
