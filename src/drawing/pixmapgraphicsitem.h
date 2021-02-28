#ifndef PIXMAPGRAPHICSITEM_H
#define PIXMAPGRAPHICSITEM_H

#include <QSet>
#include <QObject>
#include <QPainter>
#include <QGraphicsItem>

#define BLINDS_NUMBER_H 6
#define BLINDS_NUMBER_V 8
#define GLITTER_ROW 525617 // ca. GLITTER_ROW/HASH_RESOLUTION glitter pixels per row
#define GLITTER_NUMBER 137
#define HASH_RESOLUTION 7200 // in dpi

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
    /// map 100*resolution to pixmaps (resolution in dpi)
    QMap<unsigned int, QPixmap> pixmaps;
    QRectF bounding_rect;

    QRectF _mask;
    unsigned int animation_progress = UINT_MAX;
    MaskType mask_type = NoMask;
    QSet<unsigned int> newHashs;

public:
    enum {Type = UserType + 4};

    /// Trivial constructor.
    explicit PixmapGraphicsItem(const QRectF &rect, QGraphicsItem *parent = NULL) :
        QObject(NULL), QGraphicsItem(parent), bounding_rect(rect) {}

    /// return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    bool hasResolution(qreal resolution) const noexcept
    {return pixmaps.contains(HASH_RESOLUTION*resolution);}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    QRectF boundingRect() const override
    {return bounding_rect;}

    QPixmap getPixmap(qreal resolution) const noexcept;

    const QRectF mask() const noexcept
    {return _mask;}

    MaskType maskType() const noexcept
    {return mask_type;}

    int progress() const noexcept
    {return animation_progress;}

    int number() const noexcept
    {return pixmaps.size();}

public slots:
    void addPixmap(const QPixmap& pixmap, const qreal resolution);
    void addPixmap(const QPixmap& pixmap)
    {addPixmap(pixmap, pixmap.width() / boundingRect().width());}
    void setRect(const QRectF &rect) noexcept
    {bounding_rect = rect; update();}
    void setSize(const QSizeF &size) noexcept
    {bounding_rect.setSize(size);}
    void clearPixmaps() noexcept
    {pixmaps.clear();}
    void trackNew() noexcept
    {newHashs.clear();}
    void clearOld() noexcept;
    void setMask(const QRectF &rect) noexcept
    {_mask = rect; update();}
    void setMaskType(const MaskType type) noexcept;
    void setProgress(const int progress) noexcept
    {animation_progress = progress; update();}
};

#endif // PIXMAPGRAPHICSITEM_H
