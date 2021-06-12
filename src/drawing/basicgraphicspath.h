#ifndef BASICGRAPHICSPATH_H
#define BASICGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"

class QPainter;

/**
 * @class BasicGraphicsPath
 * @brief Fixed width graphics path
 * @inherits AbstractGraphicsPath
 *
 * QGraphicsItem representing a path with fixed width. See
 * AbstractGraphicsPath for details.
 */
class BasicGraphicsPath : public AbstractGraphicsPath
{
    /// Vector of nodes (coordinates).
    QVector<QPointF> data;

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + 1 };

    /// Construct path at given position with default pen.
    BasicGraphicsPath(const DrawTool &tool, const QPointF &pos) noexcept;

    /// Construct path from coordinate string.
    BasicGraphicsPath(const DrawTool &tool, const QString &coordinates) noexcept;

    /// Construct subpath of other BasicGraphicsPath, including nodes first to
    /// last-1 of other.
    BasicGraphicsPath(const BasicGraphicsPath *const other, int first, int last);

    /// return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Number of nodes of the path.
    int size() const noexcept override
    {return data.size();}

    /// Set pos to (0,0) and use absolute coordinates in data.
    void normalize() override;

    /// Position of last node in the path.
    /// Returns QPointF() if path is empty.
    const QPointF lastPoint() const noexcept override
    {return data.isEmpty() ? QPointF() : data.last();}

    /// Position of first node in the path.
    /// Returns QPointF() if path is empty.
    const QPointF firstPoint() const noexcept override
    {return data.isEmpty() ? QPointF() : data.first();}

    /// Paint this on given painter. Options and widget are currently
    /// discarded.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// Add a point to data and update bounding rect.
    void addPoint(const QPointF &point);

    /// Erase at position pos. Return a list of paths obtained when splitting
    /// this by erasing at pos with given eraser size.
    QList<AbstractGraphicsPath*> splitErase(const QPointF &eraserpos, const qreal size) override;

    /// resturn coordinates formatted as string for saving.
    const QString stringCoordinates() const noexcept override;

    /// resturn width formatted as string for saving.
    const QString stringWidth() const noexcept override
    {return QString::number(_tool.width());}
};

#endif // BASICGRAPHICSPATH_H
