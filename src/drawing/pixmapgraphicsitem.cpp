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
  debug_msg(DebugRendering, "start rendering pixmap" << this);
  if (pixmaps.isEmpty()) return;
  const QRectF rect = painter->transform().mapRect(bounding_rect);
  const QRectF viewport = painter->viewport();
  int ref_width = std::ceil(rect.width());
  int ref_height = std::ceil(rect.height());
  if (ref_width == viewport.width() + 1) ref_width -= 1;
  if (ref_height == viewport.height() + 1) ref_height -= 1;
  QPixmap pixmap;
  for (const auto &pix : pixmaps) {
    if (pix.height() >= ref_height || pix.width() >= ref_width) {
      pixmap = pix;
      break;
    }
  }
  if (pixmap.isNull()) {
    pixmap = pixmaps.last();
    qWarning() << "Showing pixmap with insufficient resolution";
  }
  if (mask_type && !_mask.isNull()) {
    switch (mask_type) {
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
  }
  painter->resetTransform();
  if (mask_type == Glitter && animation_progress != UINT_MAX) {
    const unsigned int glitter_pixel = pixmap.width() / GLITTER_ROW,
                       n = rect.width() / glitter_pixel * rect.height(),
                       w = rect.width() / glitter_pixel + 1;
    for (unsigned int j = 0; j < animation_progress; j++) {
      for (unsigned int i = shuffled(j); i < n; i += GLITTER_NUMBER) {
        painter->drawPixmap(rect.x() + glitter_pixel * (i % w),
                            rect.y() + glitter_pixel * (i / w), pixmap,
                            glitter_pixel * (i % w), glitter_pixel * (i / w),
                            glitter_pixel, glitter_pixel);
      }
    }
  } else if (rect.size() == pixmap.size())
    painter->drawPixmap(rect.topLeft(), pixmap, pixmap.rect());
  else
    painter->drawPixmap(rect, pixmap, pixmap.rect());
}

void PixmapGraphicsItem::addPixmap(const QPixmap &pixmap) noexcept
{
  if (pixmap.isNull()) return;
  auto it = pixmaps.begin();
  for (; it != pixmaps.end(); ++it) {
    if (pixmap.width() <= it->width()) break;
  }
  if (it == pixmaps.end())
    pixmaps.append(pixmap);
  else if (pixmap.width() == it->width())
    *it = pixmap;
  else
    pixmaps.insert(it, pixmap);
  newHashs.insert(pixmap.width());
  update();
}

void PixmapGraphicsItem::clearOld() noexcept
{
  for (auto it = pixmaps.begin(); it != pixmaps.end();) {
    if (newHashs.contains(it->width()))
      ++it;
    else
      it = pixmaps.erase(it);
  }
}

void PixmapGraphicsItem::setMaskType(const MaskType type) noexcept
{
  mask_type = type;
  if (mask_type == Glitter)
    reshuffle_array(pixmaps.isEmpty() ? RANDOM_INT : pixmaps.first().width());
}

QPixmap PixmapGraphicsItem::getPixmap(const unsigned int width) const noexcept
{
  for (const auto &pix : pixmaps) {
    if (pix.width() >= width) {
#ifdef QT_DEBUG
      if (pix.width() != width)
        debug_msg(DebugRendering,
                  "possibly wrong resolution:" << pix.width() << width);
#endif
      return pix;
    }
  }
  if (pixmaps.isEmpty()) return QPixmap();
  return pixmaps.last();
}

bool PixmapGraphicsItem::hasWidth(const unsigned int width) const noexcept
{
  for (const auto &pix : pixmaps) {
    if (pix.width() == width) return true;
    if (pix.width() > width) return false;
  }
  return false;
}

bool PixmapGraphicsItem::hasWidth(const qreal width) const noexcept
{
  for (const auto &pix : pixmaps) {
    if (pix.width() > width + 0.6) return false;
    if (pix.width() > width - 0.6) return true;
  }
  return false;
}
