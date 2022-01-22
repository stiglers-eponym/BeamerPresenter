#ifndef RECTGRAPHICSITEM_H
#define RECTGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include "src/drawing/basicgraphicspath.h"

/**
 * @brief RectGraphicsItem: QGraphicsRectItem adjusted for interactive drawing
 *
 * This class makes sure that the rect of the underlying QGraphicsRectItem is
 * always valid while moving one of the corners.
 */
class RectGraphicsItem : public QGraphicsRectItem
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
    enum {Type = UserType + 6};

    /// Constructor for initializing QGraphicsRectItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    RectGraphicsItem(const DrawTool &tool, const QPointF &pos, QGraphicsItem *parent = NULL);

    /// Trivial destructor.
    ~RectGraphicsItem() {}

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
        QGraphicsRectItem::paint(painter, option, widget);
    }
};

#endif // RECTGRAPHICSITEM_H
