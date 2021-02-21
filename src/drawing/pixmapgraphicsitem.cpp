#include "src/drawing/pixmapgraphicsitem.h"
#include "src/preferences.h"

#include <array>
#include <random>
#include <algorithm>


/// Generate a random array for glitter transitions.
/// Once shuffled, this array contains the numbers between 0 and GLITTER_NUMBER
/// in random order.
static std::array<unsigned int, GLITTER_NUMBER> &shuffled_array()
{
    static std::array<unsigned int, GLITTER_NUMBER> array = {GLITTER_NUMBER+1};
    if (array[0] == GLITTER_NUMBER+1)
    {
        unsigned int i=0;
        do
            array[i] = i;
        while(++i < GLITTER_NUMBER);
    }
    return array;
}

/// Shuffle the glitter array.
static void reshuffle_array()
{
    std::array<unsigned int, GLITTER_NUMBER> &array = shuffled_array();
    std::shuffle(array.begin(), array.end(), std::default_random_engine(42));
}

/// Get random value between 0 and GLITTER_NUMBER from shuffled array.
static int shuffled(const unsigned int i)
{
    return shuffled_array()[i % GLITTER_NUMBER];
}

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
            case Glitter:
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
        if (mask_type == Glitter && animation_progress != UINT_MAX)
        {
            const unsigned int glitter_pixel = bounding_rect.width() * hash / GLITTER_ROW;
            unsigned int const n = rect.width()*rect.height()/glitter_pixel, w = rect.width()/glitter_pixel+1;
            for (unsigned int j=0; j<animation_progress; j++)
            {
                for (unsigned int i=shuffled(j); i<n; i+=GLITTER_NUMBER)
                    painter->drawPixmap(rect.x()+glitter_pixel*(i%w), rect.y()+glitter_pixel*(i/w), *it, glitter_pixel*(i%w), glitter_pixel*(i/w), glitter_pixel, glitter_pixel);
            }
        }
        else
        {
            if (it.key() == hash)
                painter->drawPixmap(rect.topLeft(), *it, it->rect());
            else
                painter->drawPixmap(rect, *it, it->rect());
        }
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

void PixmapGraphicsItem::setMaskType(const MaskType type) noexcept
{
    mask_type = type;
    if (mask_type == Glitter)
        reshuffle_array();
}
