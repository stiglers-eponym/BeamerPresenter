// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/pixmapgraphicsitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <algorithm>
#include <array>
#include <iterator>
#include <random>

#include "src/log.h"

constexpr int RANDOM_INT = 42;

/// Generate a random array for glitter transitions.
/// Once shuffled, this array contains the numbers between 0 and GLITTER_NUMBER
/// in random order.
static std::array<unsigned int, GLITTER_NUMBER> &shuffled_array()
{
  static std::array<unsigned int, GLITTER_NUMBER> array = {GLITTER_NUMBER + 1};
  if (array[0] == GLITTER_NUMBER + 1) {
    unsigned int i = 0;
    do array[i] = i;
    while (++i < GLITTER_NUMBER);
  }
  return array;
}

/// Shuffle the glitter array.
static void reshuffle_array(const int seed = 42)
{
  std::array<unsigned int, GLITTER_NUMBER> &array = shuffled_array();
  std::shuffle(array.begin(), array.end(), std::default_random_engine(seed));
}

/// Get random value between 0 and GLITTER_NUMBER from shuffled array.
inline static int shuffled(const unsigned int i)
{
  return shuffled_array()[i % GLITTER_NUMBER];
}

void PixmapGraphicsItem::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
  if (pixmaps.isEmpty()) return;
  const unsigned int hash =
      painter->transform().m11() * bounding_rect.width() + 0.499;
  QMap<unsigned int, QPixmap>::const_iterator it = pixmaps.lowerBound(hash);
  if (it == pixmaps.cend()) --it;
#ifdef QT_DEBUG
  if (it.key() != hash && it.key() != hash + 1) {
    debug_msg(DebugRendering,
              "possibly wrong resolution:"
                  << it.key()
                  << painter->transform().m11() * bounding_rect.width());
    if (it != pixmaps.cbegin())
      debug_msg(DebugRendering, "possibly better:" << std::prev(it).key());
  }
#endif
  if (mask_type && !_mask.isNull()) switch (mask_type) {
      case NoMask:
      case Glitter:
        break;
      case PositiveClipping:
        painter->setClipRect(_mask);
        break;
      case NegativeClipping: {
        QPainterPath outerpath, innerpath;
        outerpath.addRect(bounding_rect);
        innerpath.addRect(_mask);
        painter->setClipPath(outerpath - innerpath);
        break;
      }
      case VerticalBlinds: {
        QPainterPath path;
        QRectF rect(_mask);
        path.addRect(rect);
        int i = 0;
        while (++i < BLINDS_NUMBER_V) {
          rect.moveLeft(rect.left() + bounding_rect.width() / BLINDS_NUMBER_V);
          path.addRect(rect);
        }
        painter->setClipPath(path);
        break;
      }
      case HorizontalBlinds: {
        QPainterPath path;
        QRectF rect(_mask);
        path.addRect(rect);
        int i = 0;
        while (++i < BLINDS_NUMBER_H) {
          rect.moveTop(rect.top() + bounding_rect.height() / BLINDS_NUMBER_H);
          path.addRect(rect);
        }
        painter->setClipPath(path);
        break;
      }
    }
  const QRectF rect = painter->transform().mapRect(bounding_rect);
  painter->resetTransform();
  if (mask_type == Glitter && animation_progress != UINT_MAX) {
    const unsigned int glitter_pixel = hash / GLITTER_ROW,
                       n = rect.width() * rect.height() / glitter_pixel,
                       w = rect.width() / glitter_pixel + 1;
    for (unsigned int j = 0; j < animation_progress; j++)
      for (unsigned int i = shuffled(j); i < n; i += GLITTER_NUMBER)
        painter->drawPixmap(rect.x() + glitter_pixel * (i % w),
                            rect.y() + glitter_pixel * (i / w), *it,
                            glitter_pixel * (i % w), glitter_pixel * (i / w),
                            glitter_pixel, glitter_pixel);
  } else if (it.key() == hash || it.key() == hash + 1)
    painter->drawPixmap(rect.topLeft(), *it, it->rect());
  else
    painter->drawPixmap(rect, *it, it->rect());
}

void PixmapGraphicsItem::addPixmap(const QPixmap &pixmap) noexcept
{
  if (!pixmap.isNull()) {
    pixmaps[pixmap.width()] = pixmap;
    newHashs.insert(pixmap.width());
  }
  update();
}

void PixmapGraphicsItem::clearOld() noexcept
{
  for (auto it = pixmaps.begin(); it != pixmaps.end();) {
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
    reshuffle_array(pixmaps.isEmpty() ? RANDOM_INT : pixmaps.lastKey());
}

QPixmap PixmapGraphicsItem::getPixmap(const unsigned int width) const noexcept
{
  if (pixmaps.isEmpty()) return QPixmap();
  QMap<unsigned int, QPixmap>::const_iterator it = pixmaps.lowerBound(width);
  if (it == pixmaps.cend()) --it;
#ifdef QT_DEBUG
  if (it.key() != width)
    debug_msg(DebugRendering,
              "possibly wrong resolution:" << it.key() << width);
#endif
  return *it;
}

bool PixmapGraphicsItem::hasWidth(const unsigned int width) const noexcept
{
  const QMap<unsigned int, QPixmap>::const_iterator it =
      pixmaps.lowerBound(width);
  return it != pixmaps.cend() && (it.key() == width || it.key() == width + 1);
}
