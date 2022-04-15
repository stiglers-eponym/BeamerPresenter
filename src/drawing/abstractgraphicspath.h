#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QGraphicsItem>
#include "src/drawing/drawtool.h"

/**
 * @brief QGraphicsItem representing a path, abstract class
 *
 * Paths consist of a vector of nodes, a bounding rectangle, and a draw tool.
 * The draw tool and the bounding rectangle coordinates are defined in
 * AbstractGraphicsPath, the vector of nodes is defined in inheriting classes.
 *
 * Coordinates are given as positions in the PDF page, measured in points
 * as floating point values. These are the same units as used in SlideScene.
 *
 * Different implementations of AbstractGraphicsPath can be distinguished by
 * their QGraphicsItem::type().
 *
 * @see BasicGraphicsPath
 * @see FullGraphicsPath
 */
class AbstractGraphicsPath : public QGraphicsItem
{
protected:
    /**
     * @brief Pen for stroking path.
     *
     * In FullGraphicsPath, pen.width is the reference width which gets
     * rescaled by the pressure of an input device.
     */
    DrawTool _tool;

    /// Vector of nodes (coordinates).
    QVector<QPointF> coordinates;

    ///@{
    /// Bounding rect coordinates
    qreal top, bottom, left, right;
    ///@}

friend class BasicGraphicsPath;
friend class ShapeRecognizer;

public:
    /// Constructor: initialize tool.
    /// @param tool tool for stroking this path
    AbstractGraphicsPath(const DrawTool tool) noexcept : _tool(tool) {}

    /// Constructor: initialize tool and coordinates.
    /// @param tool tool for stroking this path
    /// @param coordinates vector of coordinates
    AbstractGraphicsPath(const DrawTool tool, const QVector<QPointF> &coordinates) noexcept :
        _tool(tool), coordinates(coordinates) {}

    /// Bounding rectangle of the drawing (including stroke width).
    /// @return bounding rect
    virtual QRectF boundingRect() const noexcept override
    {return QRectF(left, top, right-left, bottom-top);}

    /// @return number of nodes of the path
    int size() const noexcept
    {return coordinates.size();}

    /// Coordinate of the first node in the path.
    /// @return first point coordinate
    const QPointF firstPoint() const noexcept
    {return coordinates.isEmpty() ? QPointF() : coordinates.first();}

    /// Coordinate of the last node in the path.
    /// @return last point coordinate
    const QPointF lastPoint() const noexcept
    {return coordinates.isEmpty() ? QPointF() : coordinates.last();}

    /// Transform item coordinates such that (0,0) is the center of the item.
    void toCenterCoordinates();

    /**
     * @brief Erase at position pos.
     *
     * Create list of paths obtained when erasing at position *pos* with round
     * eraser of radius *size*. This list is empty if this path is completely
     * erased.
     *
     * @param pos position of eraser (item coordinates)
     * @param size radius of eraser
     * @return list of paths after erasing (possibly empty) or {NULL} if nothing was erased.
     */
    virtual QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const = 0;

    /// @return _tool
    const DrawTool &getTool() const noexcept
    {return _tool;}

    /// Change tool in-place.
    virtual void changeTool(const DrawTool &newtool) noexcept = 0;

    /// Write nodes coordinates to string for saving.
    /// @return list of coordinates formatted as string
    virtual const QString stringCoordinates() const noexcept = 0;

    /// Write stroke width(s) to string for saving.
    /// @return single width or list of widths formatted as string
    virtual const QString stringWidth() const noexcept = 0;
};

#endif // ABSTRACTGRAPHICSPATH_H
