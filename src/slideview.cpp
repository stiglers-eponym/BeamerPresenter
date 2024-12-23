// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/slideview.h"

#include <QGestureEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>
#include <utility>

#include "src/drawing/dragtool.h"
#include "src/drawing/pixmapgraphicsitem.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/selectiontool.h"
#include "src/log.h"
#include "src/media/mediaplayer.h"
#include "src/media/mediaslider.h"
#include "src/preferences.h"
#include "src/rendering/pixcache.h"
#include "src/slidescene.h"

SlideView::SlideView(SlideScene *scene, const PixCache *cache, QWidget *parent)
    : QGraphicsView(scene, parent)
{
  setMouseTracking(true);
  setAttribute(Qt::WA_AcceptTouchEvents);
  setTransformationAnchor(AnchorViewCenter);
  grabGesture(Qt::SwipeGesture);
  grabGesture(Qt::PinchGesture);
  setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                 QPainter::RenderHint::SmoothPixmapTransform);
  setMinimumSize(4, 3);
  setFocusPolicy(Qt::StrongFocus);
  setFrameShape(QFrame::NoFrame);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  connect(this, &SlideView::requestPage, cache, &PixCache::requestPage,
          Qt::QueuedConnection);
  connect(cache, &PixCache::pageReady, this, &SlideView::pageReady,
          Qt::QueuedConnection);
  connect(this, &SlideView::resizeCache, cache, &PixCache::updateFrame,
          Qt::QueuedConnection);
  connect(this, &SlideView::getPixmapBlocking, cache, &PixCache::getPixmap,
          Qt::BlockingQueuedConnection);
}

QSize SlideView::sizeHint() const noexcept
{
  QSizeF size = scene()->sceneRect().size();
  size *= 2048. / std::max(size.width(), size.height());
  return size.toSize();
}

void SlideView::pageChanged(const int page, SlideScene *scene)
{
  sliders.clear();
  setScene(scene);
  const QSizeF &pageSize = scene->pageSize();
  if (pageSize.width() * height() > pageSize.height() * width())
    // page is too wide, determine resolution by x direction
    resolution = width() / pageSize.width();
  else
    // page is too high, determine resolution by y direction
    resolution = height() / pageSize.height();
  if (resolution < 1e-6 || resolution > 1e6) return;
  resetTransform();
  scale(resolution, resolution);
  waitingForPage = page;
  debug_msg(DebugPageChange, "Request page" << page << "by" << this << "from"
                                            << scene << "with size"
                                            << scene->pageSize() << size());
  emit requestPage(page, resolution);
}

void SlideView::pageChangedBlocking(const int page, SlideScene *scene)
{
  sliders.clear();
  setScene(scene);
  const QSizeF &pageSize = scene->pageSize();
  if (pageSize.width() * height() > pageSize.height() * width())
    // page is too wide, determine resolution by x direction
    resolution = width() / pageSize.width();
  else
    // page is too high, determine resolution by y direction
    resolution = height() / pageSize.height();
  if (resolution < 1e-6 || resolution > 1e6) return;
  resetTransform();
  scale(resolution, resolution);
  QPixmap pixmap;
  debug_msg(DebugPageChange, "Request page blocking" << page << this);
  emit getPixmapBlocking(page, pixmap, resolution);
  scene->pageBackground()->addPixmap(pixmap);
  updateScene({sceneRect()});
}

void SlideView::pageReady(const QPixmap pixmap, const int page)
{
  if (waitingForPage == page) {
    debug_msg(DebugPageChange, "page ready" << page << pixmap.size() << this);
    static_cast<SlideScene *>(scene())->pageBackground()->addPixmap(pixmap);
    waitingForPage = INT_MAX;
    updateScene({sceneRect()});
  }
}

void SlideView::resizeEvent(QResizeEvent *event)
{
  if (event->size().isNull()) return;
  emit resizeCache(event->size());
  SlideScene *slidescene = static_cast<SlideScene *>(scene());
  const int page = slidescene->getPage();
  pageChanged(page, slidescene);
  for (const auto &media : std::as_const(slidescene->getMedia()))
#if __cplusplus >= 202002L
    if (media.pages.contains(page))
#else
    if (media->pages().find(page) != media->pages().end())
#endif
      addMediaSlider(media);
  emit sendAction(ResizeViews);
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
  // Check if currenly keys should be interpreted by a graphics text item
  // or by master (as a keyboard shortcut).
  if (static_cast<const SlideScene *>(scene())->isTextEditing()) {
    switch (event->key()) {
      case Qt::Key_Escape:
        scene()->setFocusItem(nullptr);
        break;
      case Qt::Key_PageUp:
      case Qt::Key_PageDown:
        emit sendKeyEvent(event);
        break;
      default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
  } else
    emit sendKeyEvent(event);
}

int SlideView::heightForWidth(int width) const noexcept
{
  const QRectF &reference = scene()->sceneRect();
  return reference.width() * reference.height() / width;
}

const QPointF SlideView::mapToScene(const QPointF &pos) const
{
  QPointF point = pos;
  point.rx() -= viewportTransform().m31();
  point.ry() -= viewportTransform().m32();
  return point / viewportTransform().m11();
  // This is equivalent to:
  // return viewportTransform().inverted().map(pos);
}

bool SlideView::handleGestureEvent(QGestureEvent *event)
{
  const auto tool = preferences()->currentTool(Tool::TouchInput);
  QSwipeGesture *swipe =
      static_cast<QSwipeGesture *>(event->gesture(Qt::SwipeGesture));
  bool handled = false;
  if (swipe && swipe->state() == Qt::GestureFinished) {
    Gesture gesture = InvalidGesture;
    handled = true;
    if (swipe->swipeAngle() < 22.5 || swipe->swipeAngle() > 337.5)
      gesture = SwipeRight;
    else if (swipe->swipeAngle() > 247.5 && swipe->swipeAngle() < 292.5)
      gesture = SwipeDown;
    else if (swipe->swipeAngle() > 157.5 && swipe->swipeAngle() < 202.5)
      gesture = SwipeLeft;
    else if (swipe->swipeAngle() > 67.5 && swipe->swipeAngle() < 112.5)
      gesture = SwipeUp;
    else
      debug_msg(DebugOtherInput,
                "Swipe gesture ignored, angle:" << swipe->swipeAngle());
    if (gesture != InvalidGesture) {
      debug_msg(DebugOtherInput,
                "Swipe gesture, angle:" << swipe->swipeAngle()
                                        << "interpret as:" << gesture);
      const QList<Action> actions{
          preferences()->gesture_actions.values(gesture)};
      if (!actions.isEmpty()) {
        for (auto action : actions) emit sendAction(action);
        event->accept();
      }
    }
  }
  const QPinchGesture *pinch =
      dynamic_cast<QPinchGesture *>(event->gesture(Qt::PinchGesture));
  if (pinch && tool && tool->tool() == Tool::DragViewTool) {
    const auto dtool = std::dynamic_pointer_cast<DragTool>(tool);
    auto *sscene = dynamic_cast<SlideScene *>(scene());
    if (sscene && dtool && (dtool->flags() & DragTool::TouchZoom)) {
      event->accept();
      handled = true;
      dtool->clear();
      const qreal zoom = pinch->scaleFactor() * sscene->getZoom();
      debug_msg(DebugOtherInput, pinch);
      sscene->setZoom(zoom, mapToScene(pinch->centerPoint()),
                      pinch->state() == Qt::GestureFinished);
    }
  }
  return handled || QGraphicsView::event(event);
}

bool SlideView::event(QEvent *event)
{
  switch (event->type()) {
    case QEvent::Gesture:
      return handleGestureEvent(static_cast<QGestureEvent *>(event));
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletMove: {
      const auto sscene = dynamic_cast<SlideScene *>(scene());
      const auto tabletevent = dynamic_cast<const QTabletEvent *>(event);
      if (!sscene || !tabletevent) return false;
      const int device = tablet_event_to_input_device(tabletevent);
#if (QT_VERSION_MAJOR >= 6)
      const qint64 id = tabletevent->pointingDevice()->uniqueId().numericId();
      const auto pos = mapToScene(tabletevent->position());
#else
      const qint64 id = tabletevent->uniqueId();
      const auto pos = mapToScene(tabletevent->posF());
#endif
      const int old_device = active_tablet_devices.value(id, -1);
      if (id > 0 && old_device != device) {
        if (old_device > 0) sscene->tabletRelease(pos, old_device, 0);
        if (event->type() == QEvent::TabletRelease) {
          active_tablet_devices.remove(id);
          sscene->tabletRelease(pos, device, 0);
        } else {
          active_tablet_devices[id] = device;
          sscene->tabletPress(pos, device, tabletevent->pressure());
        }
      } else if (event->type() == QEvent::TabletMove) {
        sscene->tabletMove(pos, device, tabletevent->pressure());
      } else if (event->type() == QEvent::TabletPress) {
        sscene->tabletPress(pos, device, tabletevent->pressure());
      } else {
        if (id > 0) active_tablet_devices.remove(id);
        sscene->tabletRelease(pos, device, tabletevent->pressure());
      }
      event->accept();
      setFocus();
      return true;
    }
    default:
      return QGraphicsView::event(event);
  }
  return false;
}

void SlideView::requestScaledPage(const qreal zoom)
{
  const SlideScene *sscene = dynamic_cast<SlideScene *>(scene());
  if (!sscene || zoom < 1e-6 || zoom > 1e6) return;
  const PixmapGraphicsItem *pageItem = sscene->pageBackground();
  if (!pageItem) return;
  const qreal target_width = zoom * resolution * sscene->pageSize().width();
  // Check whether an enlarged page is needed and not "in preparation" yet.
  if (waitingForPage == INT_MAX && !pageItem->hasWidth(target_width)) {
    debug_msg(DebugRendering, "Enlarged page: searched for" << target_width);
    waitingForPage = sscene->getPage();
    emit requestPage(waitingForPage, zoom * resolution);
  }
}

void SlideView::showMagnifier(QPainter *painter,
                              std::shared_ptr<PointingTool> tool) noexcept
{
  painter->setRenderHints(QPainter::SmoothPixmapTransform |
                          QPainter::Antialiasing);
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter->setPen(tool->color());
  painter->setBrush(Qt::NoBrush);
  const SlideScene *sscene = dynamic_cast<SlideScene *>(scene());
  if (sscene) requestScaledPage(sscene->getZoom() * tool->scale());
  // Draw magnifier(s) at all positions of tool.
  for (const auto &pos : tool->pos()) {
    // calculate target rect: size of the magnifier
    const QRectF scene_rect(pos.x() - tool->size(), pos.y() - tool->size(),
                            2 * tool->size(), 2 * tool->size());
    // clip painter to target circle in target rect.
    QPainterPath path;
    path.addEllipse(scene_rect);
    painter->setClipPath(path);
    // fill magnifier with background color
    painter->fillPath(path, QBrush(palette().base()));
    // Calculate target rect for painter.
    QRectF target_rect({0, 0}, tool->scale() * scene_rect.size());
    target_rect.moveCenter({pos.x(), pos.y()});
    // render scene in magnifier
    scene()->render(painter, target_rect, scene_rect);
    // draw circle around magnifier
    painter->drawEllipse(pos, tool->size() - 0.5, tool->size() - 0.5);
  }
}

void SlideView::drawForeground(QPainter *painter, const QRectF &rect)
{
  if (view_flags & ShowPointingTools) {
    painter->setRenderHint(QPainter::Antialiasing);
    const auto &current_tools = preferences()->current_tools;
    const auto end = current_tools.cend();
    for (auto basic_tool = current_tools.cbegin(); basic_tool != end;
         ++basic_tool) {
      // We rely on the fact that basic_tool.key() == (*basic_tool)->tool()
      // Only pointing tools and selection tools need painting in foreground.
      if (!*basic_tool) continue;
      if (basic_tool.key() & Tool::AnyPointingTool) {
        if (!(*basic_tool)->visible()) continue;
        auto tool = std::static_pointer_cast<PointingTool>(*basic_tool);
        if (tool->pos().isEmpty() || tool->scene() != scene()) continue;
        debug_verbose(DebugDrawing, "drawing tool" << tool->tool()
                                                   << tool->size()
                                                   << tool->color());
        switch (basic_tool.key()) {
          case Tool::Torch:
            showTorch(painter, tool);
            break;
          case Tool::Eraser:
            if (hasFocus()) showEraser(painter, tool);
            break;
          case Tool::Magnifier:
            showMagnifier(painter, tool);
            break;
          case Tool::Pointer:
            showPointer(painter, tool);
            break;
          default:
            break;
        }
      } else if (basic_tool.key() & Tool::AnySelectionTool) {
        if (!(*basic_tool)->visible()) continue;
        auto tool = std::static_pointer_cast<const SelectionTool>(*basic_tool);
        if (!tool->visible() || tool->scene() != scene()) continue;
        const QPolygonF polygon = tool->polygon();
        if (polygon.length() < 3) continue;
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->setPen(preferences()->selection_rect_pen);
        painter->setBrush(preferences()->selection_rect_brush);
        painter->drawPolygon(polygon);
      }
    }
  }
#ifdef QT_DEBUG
  if ((preferences()->debug_level & (DebugMedia | DebugVerbose)) ==
      (DebugMedia | DebugVerbose)) {
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setBrush(Qt::NoBrush);
    const int page = static_cast<SlideScene *>(scene())->getPage();
    const QList<std::shared_ptr<MediaItem>> &media =
        static_cast<SlideScene *>(scene())->getMedia();
    for (const auto &m : media) {
#if __cplusplus >= 202002L
      if (m.pages.contains(page))
#else
      if (m->pages().find(page) != m->pages().end())
#endif
        painter->setPen(QPen(Qt::red, 1));
      else if (m->hasProvider())
        painter->setPen(QPen(Qt::green, 0.75));
      else
        painter->setPen(QPen(Qt::blue, 0.75));
      painter->drawRect(m->rect());
    }
  }
#endif
}

void SlideView::showEraser(QPainter *painter,
                           std::shared_ptr<PointingTool> tool) noexcept
{
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter->setPen(QPen(tool->brush(), tool->scale()));
  painter->setBrush(Qt::NoBrush);
  const float radius = tool->size() - tool->scale();
  for (const auto &pos : tool->pos()) painter->drawEllipse(pos, radius, radius);
}

void SlideView::showPointer(QPainter *painter,
                            std::shared_ptr<PointingTool> tool) noexcept
{
  painter->setCompositionMode(QPainter::CompositionMode_Darken);
  painter->setPen(Qt::PenStyle::NoPen);
  painter->setBrush(tool->brush());
  for (const auto &pos : tool->pos())
    painter->drawEllipse(pos, tool->size(), tool->size());
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
  for (const auto &pos : tool->pos())
    painter->drawEllipse(pos, tool->size(), tool->size());
}

void SlideView::showTorch(QPainter *painter,
                          std::shared_ptr<PointingTool> tool) noexcept
{
  painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter->setPen(Qt::PenStyle::NoPen);
  painter->setBrush(QBrush(tool->color(), Qt::SolidPattern));
  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  for (const auto &pos : tool->pos())
    path.addEllipse(pos, tool->size(), tool->size());
  QPainterPath fullpath;
  QRectF fullrect({-viewportTransform().m31(), -viewportTransform().m32()},
                  size());
  fullrect.setSize(fullrect.size() / viewportTransform().m11());
  fullrect.moveTo(fullrect.topLeft() / viewportTransform().m11());
  fullpath.addRect(fullrect);
  painter->fillPath(fullpath - path, tool->color());
}

void SlideView::addMediaSlider(const std::shared_ptr<MediaItem> media)
{
  // check if slider can and should be created
  if (!((view_flags & MediaControls) &&
        (media->flags() & MediaAnnotation::ShowSlider) && media->player()))
    return;
  MediaPlayer *player = media->player();
  if (!player) return;
  // create a new slider and add it to sliders
  sliders.push_back(std::unique_ptr<MediaSlider>(new MediaSlider(this)));
  const auto slider = sliders.back().get();
  // determine the slider geometry such that it is placed on the visible area
  QPoint left = mapFromScene(media->rect().bottomLeft());
  if (left.x() < 5) left.setX(5);
  const QPoint right = mapFromScene(media->rect().bottomRight());
  slider->setGeometry(left.x(), std::min(left.y(), height() - 20),
                      std::max(std::min(right.x(), width() - 5) - left.x(), 20),
                      20);
  // connect events and set properties
  connect(player, &MediaPlayer::durationChanged, slider,
          &MediaSlider::setMaximumInt64);
  connect(player, &MediaPlayer::positionChanged, slider,
          &MediaSlider::setValueInt64);
  slider->setMaximum(player->duration());
  slider->setValue(player->position());
  connect(slider, &MediaSlider::sliderMoved, player,
          &MediaPlayer::setPositionSoft);
  connect(slider, &MediaSlider::jumpTo, player, &MediaPlayer::setPosition);
  debug_msg(DebugMedia,
            "created slider:" << slider->maximum() << slider->value());
  slider->show();
}

void SlideView::prepareTransition(PixmapGraphicsItem *transitionItem)
{
  const QSizeF size = sceneRect().size() * transform().m11();
  QPixmap pixmap(std::ceil(size.width()), std::ceil(size.height()));
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);
  QRect sourceRect(mapFromScene({0, 0}), pixmap.size());
  // temporarily disable foreground painting while painting slide.
  const ViewFlags show_foreground = view_flags & ShowPointingTools;
  view_flags ^= show_foreground;
  render(&painter, pixmap.rect(), sourceRect);
  painter.end();
  view_flags ^= show_foreground;
  transitionItem->addPixmap(pixmap);
}

void SlideView::prepareFlyTransition(const bool outwards,
                                     const PixmapGraphicsItem *old,
                                     PixmapGraphicsItem *target)
{
  if (!old || !target) return;

  debug_msg(DebugTransitions, "preparing fly transition");
  const unsigned int width = transform().m11() * sceneRect().width();
  QImage newimg, oldimg;
  QPainter painter;
  if (outwards) {
    newimg = old->getPixmap(width).toImage();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    newimg.convertTo(QImage::Format_ARGB32);
#else
    newimg = newimg.convertToFormat(QImage::Format_ARGB32);
#endif
    oldimg = QImage(newimg.size(), QImage::Format_ARGB32);
    oldimg.fill(0);
    painter.begin(&oldimg);
  } else {
    oldimg = old->getPixmap(width).toImage();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    oldimg.convertTo(QImage::Format_ARGB32);
#else
    oldimg = oldimg.convertToFormat(QImage::Format_ARGB32);
#endif
    newimg = QImage(oldimg.size(), QImage::Format_ARGB32);
    newimg.fill(0);
    painter.begin(&newimg);
  }
  if (oldimg.isNull() || newimg.isNull() || oldimg.size() != newimg.size() ||
      oldimg.format() != QImage::Format_ARGB32 ||
      newimg.format() != QImage::Format_ARGB32) {
    qWarning() << "Failed to prepare fly transition";
    return;
  }
  painter.setRenderHint(QPainter::Antialiasing);
  QRect sourceRect(mapFromScene({0, 0}), newimg.size());
  // temporarily disable foreground painting while painting slide.
  const ViewFlags show_foreground = view_flags & ShowPointingTools;
  view_flags ^= show_foreground;
  render(&painter, newimg.rect(), sourceRect);
  painter.end();
  view_flags ^= show_foreground;

  unsigned char r, g, b, a;
  const QRgb *oldpixel, *end;
  QRgb *newpixel;
  for (int i = 0; i < newimg.height(); i++) {
    oldpixel = (const QRgb *)(oldimg.constScanLine(i));
    newpixel = (QRgb *)(newimg.scanLine(i));
    end = oldpixel + newimg.width();
    for (; oldpixel != end; ++oldpixel, ++newpixel) {
      if (*oldpixel == *newpixel)
        *newpixel = 0;
      else {
        /*
         * Do fancy transparency effects:
         * Make the new pixels as transparent as possible while ensuring that
         * the new page is given by adding the transparent new pixels to the
         * old pixels.
         * The requirement for r,g,b is (1-alpha)*old + alpha*diff = 255*new.
         * Here old is e.g. the red component of *oldpixel, new is the red
         * component of *newpixel before it is overwritten, and diff is what
         * we write to the red component of *newpixel in this loop.
         *
         * First determine maximum alpha from different channels:
         * r := minimum alpha required for the red channel
         * g := minimum alpha required for the green channel
         * b := minimum alpha required for the blue channel
         */
        r = (0xff0000 & *oldpixel) == (0xff0000 & *newpixel)
                // Check whether the red components of the two pixels differ.
                ? 0  // Both pixels have the same red component.
                     // New pixel could be completely transparent.
                : ((0xff0000 & *oldpixel) > (0xff0000 & *newpixel)
                       // Check which pixel has higher red component.
                       ? 255 - 255 * qRed(*newpixel) / qRed(*oldpixel)
                       // oldpixel has more red. This amount of alpha is
                       // needed if newpixel is semitransparent black.
                       : 255 * (qRed(*newpixel) - qRed(*oldpixel)) /
                             (255 - qRed(*oldpixel)));
        // newpixel has more red. This amount of alpha
        // is needed if newpixel is semitransparent white.
        g = (0x00ff00 & *oldpixel) == (0x00ff00 & *newpixel)
                ? 0
                : ((0x00ff00 & *oldpixel) > (0x00ff00 & *newpixel)
                       ? 255 - 255 * qGreen(*newpixel) / qGreen(*oldpixel)
                       : 255 * (qGreen(*newpixel) - qGreen(*oldpixel)) /
                             (255 - qGreen(*oldpixel)));
        b = (0x0000ff & *oldpixel) == (0x0000ff & *newpixel)
                ? 0
                : ((0x0000ff & *oldpixel) > (0x0000ff & *newpixel)
                       ? 255 - 255 * qBlue(*newpixel) / qBlue(*oldpixel)
                       : 255 * (qBlue(*newpixel) - qBlue(*oldpixel)) /
                             (255 - qBlue(*oldpixel)));
        // a := max(r, g, b) = minimum alpha for the pixel
        a = r > g ? (r > b ? r : b) : (g > b ? g : b);
        if (a == 0)
          // *newpixel can be fully transparent.
          *newpixel = 0;
        else if (a != 255) {
          // Calculate r,g,b components of new pixel based on the alpha value a.
          r = (255 * qRed(*newpixel) - qRed(*oldpixel) * (255 - a)) / a;
          g = (255 * qGreen(*newpixel) - qGreen(*oldpixel) * (255 - a)) / a;
          b = (255 * qBlue(*newpixel) - qBlue(*oldpixel) * (255 - a)) / a;
          *newpixel = qRgba(r, g, b, a);
        }
        // The case a==255 can be ignored, because in that case *newpixel should
        // remain unchanged.
      }
    }
  }

  debug_msg(DebugTransitions, "Prepared fly transition" << newimg.size());
  target->addPixmap(QPixmap::fromImage(newimg));
}
