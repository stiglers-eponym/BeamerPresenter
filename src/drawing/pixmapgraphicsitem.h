#ifndef PIXMAPGRAPHICSITEM_H
#define PIXMAPGRAPHICSITEM_H

#include <QSet>
#include <QObject>
#include <QGraphicsItem>

#define BLINDS_NUMBER_H 6
#define BLINDS_NUMBER_V 8
#define GLITTER_ROW 71
#define GLITTER_NUMBER 137

class QPainter;

/**
 * @brief Pixmaps for QGraphicsScene with multiple resolutions
 *
 * Multiple pixmaps for the same vector graphic at different resolutions.
 * This makes it possible to use the page background as a QGraphicsItem
 * while showing different pixmaps with the correct resolution for different
 * views of the QGraphicsScene.
 */
class PixmapGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal x READ x WRITE setX)
    Q_PROPERTY(qreal y READ y WRITE setY)
    Q_PROPERTY(QRectF mask READ mask WRITE setMask)
    Q_PROPERTY(int progress READ progress WRITE setProgress)

public:
    enum MaskType
    {
        NoMask,
        PositiveClipping,
        NegativeClipping,
        HorizontalBlinds,
        VerticalBlinds,
        Glitter,
    };

private:
    /// map pixmap width (in pixels) to pixmaps
    QMap<unsigned int, QPixmap> pixmaps;

    /// Bouding rect of this QGraphicsItem.
    QRectF bounding_rect;

    /// Rectangular mask. Depending on mask_type, painting will be clipped to mask,
    /// to the inverse of mask, or to repeated copies of mask.
    QRectF _mask;

    /// Animation progress, only relevant for glitter transition.
    /// A value of UINT_MAX means that no glitter animation is currently active.
    unsigned int animation_progress = UINT_MAX;

    /// Mask type defines meaning of mask and depends on the slide transition.
    MaskType mask_type = NoMask;

    /// List of hashs (widths) of pixmaps which were added since the latest
    /// call to trackChanges().
    QSet<unsigned int> newHashs;

public:
    enum {Type = UserType + 4};

    /// Trivial constructor.
    explicit PixmapGraphicsItem(const QRectF &rect, QGraphicsItem *parent = NULL) :
        QObject(NULL), QGraphicsItem(parent), bounding_rect(rect) {}

    /// return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    /// Check whether this contains a pixmap with the given width.
    bool hasWidth(const unsigned int width) const noexcept
    {return pixmaps.contains(width);}

    /// Paint this QGraphicsItem to painter.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// Bounding rect in scene coordinates
    QRectF boundingRect() const override
    {return bounding_rect;}

    /// Get pixmap with the given width or next larger width.
    QPixmap getPixmap(const unsigned int width) const noexcept;

    /// Get current mask.
    const QRectF mask() const noexcept
    {return _mask;}

    /// Set mask type (meaning of the mask).
    MaskType maskType() const noexcept
    {return mask_type;}

    /// Return animation progress in glitter animation.
    int progress() const noexcept
    {return animation_progress;}

    /// Return number of pixmaps.
    int number() const noexcept
    {return pixmaps.size();}

public slots:
    /// Add a pixmap.
    void addPixmap(const QPixmap& pixmap);

    /// Set (overwrite) bounding rect.
    void setRect(const QRectF &rect) noexcept
    {bounding_rect = rect; update();}

    /// Set (overwrite) bounding rect size.
    void setSize(const QSizeF &size) noexcept
    {bounding_rect.setSize(size);}

    /// Clear everything.
    void clearPixmaps() noexcept
    {pixmaps.clear();}

    /// Start tracking changes.
    void trackNew() noexcept
    {newHashs.clear();}

    /// Clear everything that was added or modified before the latest call to trackNew().
    void clearOld() noexcept;

    /// Set rectangular mask.
    void setMask(const QRectF &rect) noexcept
    {_mask = rect; update();}

    /// Set mask type (meaning of mask).
    void setMaskType(const MaskType type) noexcept;

    /// Set animation progress (only in glitter transition).
    void setProgress(const int progress) noexcept
    {animation_progress = progress; update();}
};

#endif // PIXMAPGRAPHICSITEM_H
