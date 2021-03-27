#include "src/slideview.h"
#include "src/preferences.h"
#include "src/drawing/pixmapgraphicsitem.h"
#include "src/drawing/pointingtool.h"
#include "src/rendering/pixcache.h"

SlideView::SlideView(SlideScene *scene, PixCache *cache, QWidget *parent) :
    QGraphicsView(scene, parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    setMinimumSize(4, 3);
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    cache->updateFrame(size());
    connect(this, &SlideView::requestPage, cache, &PixCache::requestPage, Qt::QueuedConnection);
    connect(cache, &PixCache::pageReady, this, &SlideView::pageReady, Qt::QueuedConnection);
    connect(this, &SlideView::resizeCache, cache, &PixCache::updateFrame, Qt::QueuedConnection);
    connect(this, &SlideView::getPixmapBlocking, cache, &PixCache::getPixmap, Qt::BlockingQueuedConnection);
}

SlideView::~SlideView() noexcept
{
    qDeleteAll(sliders);
    sliders.clear();
}

void SlideView::pageChanged(const int page, SlideScene *scene)
{
    qDeleteAll(sliders);
    sliders.clear();
    setScene(scene);
    const QSizeF &pageSize = scene->sceneRect().size();
    qreal resolution;
    if (pageSize.width() * height() > pageSize.height() * width())
        // page is too wide, determine resolution by x direction
        resolution = width() / pageSize.width();
    else
        // page is too high, determine resolution by y direction
        resolution = height() / pageSize.height();
    if (resolution < 1e-9 || resolution > 1e9)
        return;
    resetTransform();
    scale(resolution, resolution);
    waitingForPage = page;
    debug_msg(DebugPageChange) << "Request page" << page << "by" << this << "from" << scene;
    emit requestPage(page, resolution);
}

void SlideView::pageChangedBlocking(const int page, SlideScene *scene)
{
    qDeleteAll(sliders);
    sliders.clear();
    setScene(scene);
    const QSizeF &pageSize = scene->sceneRect().size();
    qreal resolution;
    if (pageSize.width() * height() > pageSize.height() * width())
        // page is too wide, determine resolution by x direction
        resolution = width() / pageSize.width();
    else
        // page is too high, determine resolution by y direction
        resolution = height() / pageSize.height();
    if (resolution < 1e-9 || resolution > 1e9)
        return;
    resetTransform();
    scale(resolution, resolution);
    QPixmap pixmap;
    debug_msg(DebugPageChange) << "Request page blocking" << page << this;
    emit getPixmapBlocking(page, &pixmap, resolution);
    scene->pageBackground()->addPixmap(pixmap);
    updateScene({sceneRect()});
}

void SlideView::pageReady(const QPixmap pixmap, const int page)
{
    if (waitingForPage == page)
    {
        debug_msg(DebugPageChange) << "page ready" << page << pixmap.size() << this;
        static_cast<SlideScene*>(scene())->pageBackground()->addPixmap(pixmap);
        waitingForPage = INT_MAX;
        updateScene({sceneRect()});
    }
}

void SlideView::resizeEvent(QResizeEvent *event)
{
    if (event->size().isNull())
        return;
    emit resizeCache(event->size());
    SlideScene *slidescene = static_cast<SlideScene*>(scene());
    pageChanged(slidescene->getPage(), slidescene);
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
    // Check if currenly keys should be interpreted by a graphics text item
    // or by master (as a keyboard shortcut).
    if (static_cast<const SlideScene*>(scene())->isTextEditing())
    {
        switch (event->key())
        {
        case Qt::Key_Escape:
            scene()->clearFocus();
            break;
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            emit sendKeyEvent(event);
            break;
        default:
            QGraphicsView::keyPressEvent(event);
        }
    }
    else
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
    //return viewportTransform().inverted().map(pos);
}

bool SlideView::event(QEvent *event)
{
    switch (event->type())
    {
    //case QEvent::TabletTrackingChange:
    //case QEvent::TabletEnterProximity:
    //case QEvent::TabletLeaveProximity:
    //    break;
    case QEvent::TabletPress:
    {
        auto tabletevent = static_cast<QTabletEvent*>(event);
        static_cast<SlideScene*>(scene())->tabletPress(mapToScene(tabletevent->posF()), tabletevent);
        event->accept();
        return true;
    }
    case QEvent::TabletRelease:
    {
        auto tabletevent = static_cast<QTabletEvent*>(event);
        static_cast<SlideScene*>(scene())->tabletRelease(mapToScene(tabletevent->posF()), tabletevent);
        event->accept();
        return true;
    }
    case QEvent::TabletMove:
    {
        auto tabletevent = static_cast<QTabletEvent*>(event);
        static_cast<SlideScene*>(scene())->tabletMove(mapToScene(tabletevent->posF()), tabletevent);
        event->accept();
        return true;
    }
    default:
        return QGraphicsView::event(event);
    }
}

void SlideView::showMagnifier(QPainter *painter, const PointingTool *tool) noexcept
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    painter->setRenderHints(QPainter::Antialiasing);
    painter->setPen(tool->color());
    const qreal resolution = tool->scale() / painter->transform().m11();
    PixmapGraphicsItem *pageItem = static_cast<SlideScene*>(scene())->pageBackground();
    // Check whether an enlarged page is needed and not "in preparation" yet.
    if (waitingForPage == INT_MAX && !pageItem->hasWidth(resolution*sceneRect().width() + 0.5))
    {
        const int page = static_cast<SlideScene*>(scene())->getPage();
        const QSizeF &pageSize = scene()->sceneRect().size();
        if (!pageSize.isNull())
        {
            debug_msg(DebugDrawing) << "Request enlarged page" << page << pageSize << this;
            waitingForPage = page;
            emit requestPage(page,
                                tool->scale() * (
                                    (pageSize.width() * height() > pageSize.height() * width()) ?
                                    width() / pageSize.width() :
                                    height() / pageSize.height()
                                )
                            );
        }
    }
    // Draw magnifier(s) at all positions of tool.
    for (const auto &pos : tool->pos())
    {
        // calculate target rect: size of the magnifier
        const QRectF scene_rect(pos.x()-tool->size(), pos.y()-tool->size(), 2*tool->size(), 2*tool->size());
        // clip painter to target circle in target rect.
        QPainterPath path;
        path.addEllipse(scene_rect);
        painter->setClipPath(path);
        // fill magnifier with background color
        painter->fillPath(path, QBrush(palette().base()));
        // Calculate target rect for painter.
        QRectF target_rect({0,0}, tool->scale()*scene_rect.size());
        target_rect.moveCenter({pos.x(), pos.y()});
        // render scene in magnifier
        scene()->render(painter, target_rect, scene_rect);
        // draw circle around magnifier
        painter->drawEllipse(pos, tool->size(), tool->size());
    }
}

void SlideView::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (view_flags & ShowPointingTools)
    {
        painter->setRenderHint(QPainter::Antialiasing);
        for (const auto basic_tool : preferences()->current_tools)
        {
            // Only pointing tools need painting in foreground (might change in the future).
            if (!(basic_tool->tool() & Tool::AnyPointingTool))
                continue;
            const PointingTool *tool = static_cast<PointingTool*>(basic_tool);
            if (tool->pos().isEmpty())
                continue;
            debug_verbose(DebugDrawing) << "drawing tool" << tool->tool() << tool->size() << tool->color();
            switch (tool->tool())
            {
            case Tool::Pointer:
                showPen(painter, tool);
                break;
            case Tool::Torch:
                showTorch(painter, tool);
                break;
            case Tool::Magnifier:
                showMagnifier(painter, tool);
                break;
            default:
                break;
            }
        }
    }
}

void SlideView::showPen(QPainter *painter, const PointingTool *tool) noexcept
{
    painter->setPen(Qt::PenStyle::NoPen);
    painter->setBrush(tool->brush());
    painter->setCompositionMode(QPainter::CompositionMode_Darken);
    for (const auto &pos : tool->pos())
        painter->drawEllipse(pos, tool->size(), tool->size());
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    for (const auto &pos : tool->pos())
        painter->drawEllipse(pos, tool->size(), tool->size());
}

void SlideView::showTorch(QPainter *painter, const PointingTool *tool) noexcept
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setPen(Qt::PenStyle::NoPen);
    painter->setBrush(QBrush(tool->color(), Qt::SolidPattern));
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for (const auto &pos : tool->pos())
        path.addEllipse(pos, tool->size(), tool->size());
    QPainterPath fullpath;
    QRectF fullrect({-viewportTransform().m31(), -viewportTransform().m32()}, size());
    fullrect.setSize(fullrect.size()/viewportTransform().m11());
    fullrect.moveTo(fullrect.topLeft()/viewportTransform().m11());
    fullpath.addRect(fullrect);
    painter->fillPath(fullpath - path, tool->color());
}

void SlideView::addMediaSlider(const SlideScene::VideoItem &video)
{
    if (!(view_flags & MediaControls))
        return;
    QSlider *slider = new QSlider(Qt::Horizontal, this);
    sliders.append(slider);
    const QPoint left = mapFromScene(video.annotation.rect.bottomLeft());
    const QPoint right = mapFromScene(video.annotation.rect.bottomRight());
    slider->setGeometry(left.x(), right.y(), right.x() - left.x(), 20);
    slider->setMaximum(video.player->duration());
    slider->setValue(video.player->position());
    connect(video.player, &QMediaPlayer::durationChanged, slider, &QSlider::setMaximum);
    connect(video.player, &QMediaPlayer::positionChanged, slider, &QSlider::setValue);
    connect(slider, &QSlider::sliderMoved, video.player, &QMediaPlayer::setPosition);
    QPalette palette;
    palette.setColor(QPalette::Base, QColor(0,0,0,0));
    slider->setPalette(palette);
    slider->show();
}

void SlideView::prepareTransition(PixmapGraphicsItem *transitionItem)
{
    const qreal resolution = transform().m11();
    QPixmap pixmap((sceneRect().size()*resolution).toSize());
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect sourceRect(mapFromScene({0,0}), pixmap.size());
    render(&painter, pixmap.rect(), sourceRect);
    painter.end();
    transitionItem->addPixmap(pixmap);
}

void SlideView::prepareFlyTransition(const bool outwards, const PixmapGraphicsItem *old, PixmapGraphicsItem *target)
{
    if (!old || !target)
        return;

    const unsigned int width = transform().m11() * sceneRect().width() + 0.5;
    QImage newimg, oldimg;
    QPainter painter;
    if (outwards)
    {
        newimg = old->getPixmap(width).toImage();
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        newimg.convertTo(QImage::Format_ARGB32);
#else
        newimg = newimg.convertToFormat(QImage::Format_ARGB32);
#endif
        oldimg = QImage(newimg.size(), QImage::Format_ARGB32);
        painter.begin(&oldimg);
    }
    else
    {
        oldimg = old->getPixmap(width).toImage();
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        oldimg.convertTo(QImage::Format_ARGB32);
#else
        oldimg = oldimg.convertToFormat(QImage::Format_ARGB32);
#endif
        newimg = QImage(oldimg.size(), QImage::Format_ARGB32);
        painter.begin(&newimg);
    }
    if (oldimg.isNull())
    {
        qWarning() << "Failed to prepare fly transition";
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing);
    QRect sourceRect(mapFromScene({0,0}), newimg.size());
    render(&painter, newimg.rect(), sourceRect);
    painter.end();

    unsigned char r, g, b, a;
    const QRgb *oldpixel, *end;
    QRgb *newpixel;
    for (int i=0; i<newimg.height(); i++)
    {
        oldpixel = (const QRgb*)(oldimg.constScanLine(i));
        newpixel = (QRgb*)(newimg.scanLine(i));
        end = oldpixel + newimg.width();
        for (; oldpixel != end; ++oldpixel, ++newpixel)
        {
            if (*oldpixel == *newpixel)
                *newpixel = 0;
            else
            {
                // Do fancy transparency effects:
                // Make the new pixels as transparent as possible while ensuring that the new page is given by adding the transparent new pixels to the old pixels.
                // The requirement for r,g,b is (1-alpha)/255)*old + alpha*diff = new.
                // The result (alpha,diff) is used to overwrite newimg.
                // Determine minimum alpha from different channels:
                // r := minimum alpha required for the red channel
                // g := minimum alpha required for the green channel
                // b := minimum alpha required for the blue channel
                r = (0xff0000 & *oldpixel) == (0xff0000 & *newpixel)
                        ? qRed(*newpixel)
                        : ((0xff0000 & *oldpixel) > (0xff0000 & *newpixel)
                           ? 255 - 255*qRed(*newpixel)/qRed(*oldpixel)
                           : 255*(qRed(*newpixel)-qRed(*oldpixel))/(255-qRed(*oldpixel)));
                g = (0x00ff00 & *oldpixel) == (0x00ff00 & *newpixel)
                        ? qGreen(*newpixel)
                        : ((0x00ff00 & *oldpixel) > (0x00ff00 & *newpixel)
                           ? 255 - 255*qGreen(*newpixel)/qGreen(*oldpixel)
                           : 255*(qGreen(*newpixel)-qGreen(*oldpixel))/(255-qGreen(*oldpixel)));
                b = (0x0000ff & *oldpixel) == (0x0000ff & *newpixel)
                        ? qBlue(*newpixel)
                        : ((0x0000ff & *oldpixel) > (0x0000ff & *newpixel)
                           ? 255 - 255*qBlue(*newpixel)/qBlue(*oldpixel)
                           : 255*(qBlue(*newpixel)-qBlue(*oldpixel))/(255-qBlue(*oldpixel)));
                // a := max(r, g, b) = minimum alpha/255 for the pixel
                a = r > g ? (r > b ? r : b ) : (g > b ? g : b);
                if (a == 0)
                    *newpixel = 0;
                else if (a != 255)
                {
                    r = (255*qRed(*newpixel) - qRed(*oldpixel)*(255-a))/a;
                    g = (255*qGreen(*newpixel) - qGreen(*oldpixel)*(255-a))/a;
                    b = (255*qBlue(*newpixel) - qBlue(*oldpixel)*(255-a))/a;
                    *newpixel = (a << 24) + (r << 16) + (g << 8) + b;
                }
            }
        }
    }

    debug_msg(DebugTransitions) << "Prepared fly transition" << newimg.size();
    target->addPixmap(QPixmap::fromImage(newimg));
}
