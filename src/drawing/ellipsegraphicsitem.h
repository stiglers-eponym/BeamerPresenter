#ifndef ELLIPSEGRAPHICSITEM_H
#define ELLIPSEGRAPHICSITEM_H

#include <QGraphicsEllipseItem>
#include "src/drawing/basicgraphicspath.h"

class EllipseGraphicsItem : public QGraphicsEllipseItem
{
    enum {
        TopLeft = 0x0,
        TopRight = 0x1,
        BottomLeft = 0x2,
        BottomRight = 0x3,
    };
    /// DrawTool for this path.
    DrawTool tool;
    /// Defines which corner of the rectangle is kept fixed.
    quint8 origin = TopLeft;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 7};

    /// Constructor for initializing QGraphicsRectItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    EllipseGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = NULL);

    /// Trivial destructor.
    ~EllipseGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the rectangle.
    /// Make sure that the underlying rect is always valid.
    void setSecondPoint(const QPointF &pos);

    /// Convert to a BasicGraphicsPath for simpler erasing.
    BasicGraphicsPath *toPath() const;
};

#endif // ELLIPSEGRAPHICSITEM_H
