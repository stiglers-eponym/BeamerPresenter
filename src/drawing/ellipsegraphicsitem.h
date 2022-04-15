#ifndef ELLIPSEGRAPHICSITEM_H
#define ELLIPSEGRAPHICSITEM_H

#include <QGraphicsEllipseItem>
#include "src/drawing/basicgraphicspath.h"

/**
 * @brief EllipseGraphicsItem: QGraphicsEllipseItem for flexible drawing of an ellipse
 *
 * This extends QGraphicsEllipseItem by checking that the rect is always valid.
 * It also contains a DrawTool and can be converted to a BasicGraphicsPath.
 */
class EllipseGraphicsItem : public QGraphicsEllipseItem
{
    enum {
        OriginRight = 0x1,
        OriginBottom = 0x2,
        TopLeft = 0x0,
        TopRight = OriginRight,
        BottomLeft = OriginBottom,
        BottomRight = OriginRight | OriginBottom,
    };
    /// DrawTool for this path.
    const DrawTool tool;
    /// Defines which corner of the rectangle is kept fixed.
    quint8 origin = TopLeft;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 7};

    /// Constructor for initializing QGraphicsEllipseItem
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

    /// Paint line to painter.
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override
    {
        painter->setCompositionMode(tool.compositionMode());
        QGraphicsEllipseItem::paint(painter, option, widget);
    }
};

#endif // ELLIPSEGRAPHICSITEM_H
