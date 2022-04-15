#ifndef FULLGRAPHICSPATH_H
#define FULLGRAPHICSPATH_H

#include "src/drawing/abstractgraphicspath.h"

/**
 * @brief Variable width graphics path
 *
 * QGraphicsItem representing a path with flexible width.
 * Each node of the graph contains not only coordinates, but also the width
 * for the connection to the next node.
 *
 * @see AbstractGraphicsPath
 */
class FullGraphicsPath : public AbstractGraphicsPath
{
private:
    /// Vector of pressures (for each stroke segment).
    /// coordinates and pressures must always have the same length.
    QVector<float> pressures;

friend class ShapeRecognizer;

public:
    /// Custom type of QGraphicsItem.
    enum { Type = UserType + 2 };

    /// Construct path starting at given position.
    /// @param tool draw tool for stroking path
    /// @param pos first node (coordinate) in this path
    /// @param pressure pen pressure at first node
    FullGraphicsPath(const DrawTool &tool, const QPointF &pos, const float pressure);

    /// Construct path from coordinate string and widths.
    /// @param tool draw tool for stroking path
    /// @param coordinates string representing nodes as space separated numbers
    ///        in the form x1 y1 x2 y2 x3 y3 ...
    /// @param widths string representing stroke widths in points as space
    ///        separated list in the form w1 w2 w3 ...
    FullGraphicsPath(const DrawTool &tool, const QString &coordinate_string, const QString &widths);

    /// Construct subpath of other FullGraphicsPath, including nodes first to
    /// last-1 of other.
    /// @param other other graphics path from which a subpath should be created
    /// @param first index of first node contained in the subpath.
    /// @param last index of first node after the subpath.
    FullGraphicsPath(const FullGraphicsPath *const other, int first, int last);

    /// Copy this.
    AbstractGraphicsPath *copy() const override;

    /// @return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Paint this on given painter. Options and widget are currently
    /// discarded.
    /// @param painter paint to this painter.
    /// @param option currently ignored.
    /// @param widget currently ignored.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// Add a point to data and update bounding rect.
    /// @param point new node
    /// @param pressure pen pressure at next node
    void addPoint(const QPointF &point, const float pressure);

    QList<AbstractGraphicsPath*> splitErase(const QPointF &pos, const qreal size) const override;

    /// Change width in-place.
    /// @param newwidth new tool width
    void changeWidth(const float newwidth) noexcept;

    void changeTool(const DrawTool &newtool) noexcept override;

    /// Write stroke widths to string for saving.
    /// @return space separated list of widths of the lines
    const QString stringWidth() const noexcept override;
};

#endif // FULLGRAPHICSPATH_H
