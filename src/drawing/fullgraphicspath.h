#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"

/**
 * @class FullGraphicsPath
 * @brief Variable width graphics path
 * @inherits AbstractGraphicsPath
 *
 * QGraphicsItem representing a path with flexible width.
 * Each node of the graph contains not only coordinates, but also the width
 * for the connection to the next node.
 * See AbstractGraphicsPath for details.
 */
class FullGraphicsPath : public AbstractGraphicsPath
{
public:
    /// Coordinates and pen pressure representing node of FullGraphicsPath.
    struct PointPressure
    {
        /// x coordinate in points
        qreal x;
        /// y coordinate in points
        qreal y;
        /// stroke width in points
        float pressure;
        /// QPointF representation of coordinates
        QPointF point() const {return {x,y};}
    };


private:
    /// Vector of nodes (coordinates and pressure).
    QVector<PointPressure> data;

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + 2 };

    /// Construct path with given initial node and default pen.
    FullGraphicsPath(const DrawTool &tool, const QPointF &pos, const float pressure);

    /// Construct path from string representation of coordinates and width.
    FullGraphicsPath(const DrawTool &tool, const QString &coordinates, const QString &widths);

    /// Construct subpath of other FullGraphicsPath, including nodes first to
    /// last-1 of other.
    FullGraphicsPath(const FullGraphicsPath *const other, int first, int last);

    /// return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Number of nodes of the path.
    int size() const noexcept override
    {return data.size();}

    /// Position of last node in the path.
    /// Returns QPointF() if path is empty.
    const QPointF lastPoint() const noexcept override
    {return data.isEmpty() ? QPointF() : data.last().point();}

    /// Position of first node in the path.
    /// Returns QPointF() if path is empty.
    const QPointF firstPoint() const noexcept override
    {return data.isEmpty() ? QPointF() : data.first().point();}

    /// Paint this on given painter. Options and widget are currently
    /// discarded.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point, const float pressure);

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;

    /// Change width in-place.
    void changeWidth(const float newwidth) noexcept;

    /// Change tool in-place.
    void changeTool(const DrawTool &newtool) noexcept;

    /// resturn coordinates formatted as string for saving.
    const QString stringCoordinates() const noexcept override;

    /// resturn width formatted as string for saving.
    const QString stringWidth() const noexcept override;
};

#endif // FULLGRAPHICSPATH_H
