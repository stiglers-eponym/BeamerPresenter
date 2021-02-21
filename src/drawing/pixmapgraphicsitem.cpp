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
        if (!_mask.isNull())
            switch (mask_type)
            {
            case NoMask:
                break;
            case PositiveClipping:
                painter->setClipRect(_mask);
                break;
            case NegativeClipping:
            {
                QPainterPath outerpath, innerpath;
                outerpath.addRect(bounding_rect);
                innerpath.addRect(_mask);
                painter->setClipPath(outerpath - innerpath);
                break;
            }
            case VerticalBlinds:
            {
                QPainterPath path;
                QRectF rect(_mask);
                path.addRect(rect);
                int i=0;
                while (++i < BLINDS_NUMBER_V)
                {
                    rect.moveLeft(rect.left() + bounding_rect.width()/BLINDS_NUMBER_V);
                    path.addRect(rect);
                }
                painter->setClipPath(path);
                break;
            }
            case HorizontalBlinds:
            {
                QPainterPath path;
                QRectF rect(_mask);
                path.addRect(rect);
                int i=0;
                while (++i < BLINDS_NUMBER_H)
                {
                    rect.moveTop(rect.top() + bounding_rect.height()/BLINDS_NUMBER_H);
                    path.addRect(rect);
                }
                painter->setClipPath(path);
                break;
            }
        }
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
}
