#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QGraphicsItem>
#include "src/drawing/drawtool.h"

/**
 * @class AbstractGraphicsPath
 * @brief QGraphicsItem representing a path, abstract class
 *
 * Paths consist of a vector of nodes, a bounding rectangle, and a draw tool.
 * The draw tool and the bounding rectangle coordinates are defined in
 * AbstractGraphicsPath, the vector of nodes is defined in inheriting classes.
 *
 * Coordinates are given as positions in the PDF page, measured in points
 * as floating point values. These are the same units as used in SlideScene.
 */
class AbstractGraphicsPath : public QGraphicsItem
{
protected:
    /**
     * Pen for stroking path.
     * In FullGraphicsPath, pen.width is the reference width which gets
     * rescaled by the pressure of an input device.
     */
    DrawTool _tool;

    /// Bounding rect coordinates.
    qreal top, bottom, left, right;

public:
    /// Constructor: initializing tool.
    AbstractGraphicsPath(const DrawTool tool) noexcept : _tool(tool) {}

    /// Bounding rectangle of the drawing (including stroke width).
    virtual QRectF boundingRect() const noexcept override
    {return QRectF(left, top, right-left, bottom-top);}

    /// Number of nodes of the path.
    virtual int size() const noexcept = 0;

    /// Coordinate of the first node in the path.
    virtual const QPointF firstPoint() const noexcept = 0;

    /// Coordinate of the last node in the path.
    virtual const QPointF lastPoint() const noexcept = 0;

    /**
     * @brief Erase at position pos.
     *
     * Create list of paths obtained when erasing at position <pos> with round
     * eraser of radius <size>. This list is empty if this path is completely
     * erased.
     *
     * @return QList<AbstractGraphicsPath*>, new paths after erasing (possibly
     * empty) or {NULL} if nothing was erased.
     */
    virtual QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const = 0;

    /**
     * @param tool
     */
    const DrawTool &getTool() const noexcept
    {return _tool;}

    /// resturn coordinates formatted as string for saving.
    virtual const QString stringCoordinates() const noexcept = 0;

    /// resturn width formatted as string for saving.
    virtual const QString stringWidth() const noexcept = 0;
};

#endif // ABSTRACTGRAPHICSPATH_H
