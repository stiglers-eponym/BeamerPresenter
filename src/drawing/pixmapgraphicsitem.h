#ifndef PIXMAPGRAPHICSITEM_H
#define PIXMAPGRAPHICSITEM_H

#include <QPainter>
#include <QSet>
#include <QGraphicsItem>

/**
 * @brief Pixmaps for QGraphicsScene with multiple resolutions
 *
 * Multiple pixmaps for the same vector graphic at different resolutions.
 * This makes it possible to use the page background as a QGraphicsItem
 * while showing different pixmaps with the correct resolution for different
 * views of the QGraphicsScene.
 */
class PixmapGraphicsItem : public QGraphicsItem
{
    /// map 1000*resolution to pixmaps (resolution in dpi)
    QMap<unsigned int, QPixmap> pixmaps;
    QRectF bounding_rect;
    QSet<unsigned int> newHashs;

public:
    enum {Type = UserType + 4};

    /// Trivial constructor.
    PixmapGraphicsItem(const QRectF &rect, QGraphicsItem *parent = NULL) : QGraphicsItem(parent), bounding_rect(rect) {}

    /// return custom type of QGraphicsItem.
    int type() const noexcept override
    {return Type;}

    bool hasResolution(qreal resolution) const noexcept
    {return pixmaps.contains(7200*resolution);}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *opeion, QWidget *widget = NULL) override;

    QRectF boundingRect() const override
    {return bounding_rect;}

    const QPixmap getPixmap(qreal resolution) const noexcept
    {return pixmaps.value(7200*resolution);}

public slots:
    void addPixmap(const QPixmap& pixmap);
    void setRect(const QRectF &rect) noexcept
    {bounding_rect = rect;}
    void setSize(const QSizeF &size) noexcept
    {bounding_rect.setSize(size);}
    void clearPixmaps() noexcept
    {pixmaps.clear();}
    void trackNew() noexcept
    {newHashs.clear();}
    void clearOld() noexcept;
};

#endif // PIXMAPGRAPHICSITEM_H
