#ifndef RECTGRAPHICSITEM_H
#define RECTGRAPHICSITEM_H

#include <QGraphicsRectItem>

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
    /// Defines which corner of the rectangle is kept fixed.
    quint8 origin = TopLeft;

public:
    /// QGraphicsItem type for this subclass
    enum {Type = UserType + 6};

    /// Constructor for initializing QGraphicsRectItem
    /// @param pos origin of the rectangle. This coordinate is always fixed.
    RectGraphicsItem(const QPointF &pos, QGraphicsItem *parent = NULL) : QGraphicsRectItem(pos.x(), pos.y(), 0, 0, parent) {}

    /// Trivial destructor.
    ~RectGraphicsItem() {}

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Change the flexible coordinate of the rectangle.
    /// Make sure that the underlying rect is always valid.
    void setSecondPoint(const QPointF &pos)
    {
        QRectF newrect = rect();
        switch (origin)
        {
        case TopLeft:
            newrect.setBottomRight(pos);
            break;
        case TopRight:
            newrect.setBottomLeft(pos);
            break;
        case BottomLeft:
            newrect.setTopRight(pos);
            break;
        case BottomRight:
            newrect.setTopLeft(pos);
            break;
        }
        if (newrect.width() < 0)
            origin ^= 0x1;
        if (newrect.height() < 0)
            origin ^= 0x2;
        setRect(newrect.normalized());
    }
};

#endif // RECTGRAPHICSITEM_H
