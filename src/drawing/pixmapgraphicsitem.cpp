#include "src/drawing/pixmapgraphicsitem.h"
#include "src/preferences.h"

void PixmapGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!pixmaps.isEmpty())
    {
        const unsigned int hash = 7200 * painter->transform().m11();
        QMap<unsigned int, QPixmap>::const_iterator it = pixmaps.lowerBound(hash);
        if (it == pixmaps.cend())
            --it;
        const QRectF rect = painter->transform().mapRect(bounding_rect);
        painter->resetTransform();
        if (it.key() == hash)
            painter->drawPixmap(rect.topLeft(), *it, it->rect());
        else
            painter->drawPixmap(rect, *it, it->rect());
    }
}

void PixmapGraphicsItem::addPixmap(const QPixmap &pixmap)
{
    if (!pixmap.isNull())
    {
        const int hash = 7200 * pixmap.width() / boundingRect().width();
        pixmaps[hash] = pixmap;
        newHashs.insert(hash);
    }
}

void PixmapGraphicsItem::clearOld() noexcept
{
    for (auto it = pixmaps.begin(); it != pixmaps.end();)
    {
        if (newHashs.contains(it.key()))
            ++it;
        else
            it = pixmaps.erase(it);
    }
    newHashs.clear();
}
