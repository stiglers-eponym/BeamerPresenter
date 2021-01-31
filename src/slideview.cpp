#include "src/slideview.h"
#include "src/rendering/pixcache.h"
#include "src/slidescene.h"
#include "src/preferences.h"
#include "src/pdfmaster.h"

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
}

void SlideView::pageChanged(const int page, SlideScene *scene)
{
    setScene(scene);
    const QSizeF &pageSize = scene->sceneRect().size();
    qreal resolution;
    if (pageSize.width() * height() > pageSize.height() * width())
        // page is too wide, determine resolution by x direction
        resolution = width() / pageSize.width();
    else
        // page is too high, determine resolution by y direction
        resolution = height() / pageSize.height();
    resetTransform();
    scale(resolution, resolution);
    waitingForPage = page;
    enlargedPixmap = QPixmap();
    debug_msg(DebugPageChange) << "Request page" << page << this;
    emit requestPage(page, resolution);
}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawPixmap(scene()->sceneRect(), currentPixmap, currentPixmap.rect());
}

void SlideView::pageReady(const QPixmap pixmap, const int page)
{
    debug_msg(DebugPageChange) << "page ready" << page << pixmap.size() << this;
    if (waitingForPage == page)
    {
        currentPixmap = pixmap;
        waitingForPage = INT_MAX;
        updateScene({sceneRect()});
    }
    else if (waitingForPage == -page-1)
    {
        enlargedPixmap = pixmap;
        waitingForPage = INT_MAX;
        updateScene({sceneRect()});
    }
}

void SlideView::resizeEvent(QResizeEvent *event)
{
    emit resizeCache(event->size());
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
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

void SlideView::showMagnifier(QPainter *painter, const PointingTool *tool)
{
    if (tool->pos().isNull())
        return;
    const QRectF scene_rect(tool->pos().x()-tool->size(), tool->pos().y()-tool->size(), 2*tool->size(), 2*tool->size());
    QPainterPath path;
    path.addEllipse(scene_rect);
    painter->setClipPath(path);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    if (enlargedPixmap.isNull())
    {
        if (waitingForPage == INT_MAX)
        {
            const int page = static_cast<SlideScene*>(scene())->getPage();
            const QSizeF &pageSize = scene()->sceneRect().size();
            debug_msg(DebugDrawing) << "Request enlarged page" << page << this;
            waitingForPage = -page- 1;
            emit requestPage(page,
                                tool->scale() * (
                                    (pageSize.width() * height() > pageSize.height() * width()) ?
                                    width() / pageSize.width() :
                                    height() / pageSize.height()
                                )
                            );
        }
        const qreal scale = currentPixmap.width() / sceneRect().width();
        QRectF pixmap_rect(tool->pos().x()-tool->size()/tool->scale(), tool->pos().y()-tool->size()/tool->scale(), tool->size()*2/tool->scale(), tool->size()*2/tool->scale());
        pixmap_rect.setRect(scale*pixmap_rect.x(), scale*pixmap_rect.y(), scale*pixmap_rect.width(), scale*pixmap_rect.height());
        painter->drawPixmap(scene_rect, currentPixmap, pixmap_rect);
    }
    else
    {
        const qreal scale = enlargedPixmap.width() / sceneRect().width();
        QRectF pixmap_rect(tool->pos().x()-tool->size()/tool->scale(), tool->pos().y()-tool->size()/tool->scale(), tool->size()*2/tool->scale(), tool->size()*2/tool->scale());
        pixmap_rect.setRect(scale*pixmap_rect.x(), scale*pixmap_rect.y(), scale*pixmap_rect.width(), scale*pixmap_rect.height());
        painter->drawPixmap(scene_rect, enlargedPixmap, pixmap_rect);
    }
    painter->save();
    //painter->setTransform(QTransform::fromScale(tool->scale(),tool->scale()).translate(-tool->pos().x()*(1-1/tool->scale()), -tool->pos().y()*(1-1/tool->scale())), true);
    painter->setTransform(QTransform::fromTranslate(tool->pos().x()*(1-tool->scale()), tool->pos().y()*(1-tool->scale())).scale(tool->scale(),tool->scale()), true);
    for (const auto item : static_cast<const QList<QGraphicsItem*>>(items()))
        item->paint(painter, NULL, this);
    painter->restore();
}

void SlideView::drawForeground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing);
    for (const auto basic_tool : preferences()->current_tools)
    {
        if (!(basic_tool->tool() & AnyPointingTool))
            continue;
        const PointingTool *tool = static_cast<PointingTool*>(basic_tool);
        debug_verbose(DebugDrawing) << "drawing tool" << tool->pos() << tool->tool() << tool->size() << tool->color();
        if (tool->pos().isNull())
            continue;
        switch (tool->tool())
        {
        case Pointer:
        {
            painter->setCompositionMode(QPainter::CompositionMode_Darken);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->setBrush(QBrush(tool->color(), Qt::SolidPattern));
            painter->drawEllipse(tool->pos(), tool->size(), tool->size());
            break;
        }
        case Torch:
        {
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setPen(Qt::PenStyle::NoPen);
            painter->setBrush(QBrush(tool->color(), Qt::SolidPattern));
            QPainterPath path;
            path.addRect(sceneRect());
            path.addEllipse(tool->pos(), tool->size(), tool->size());
            painter->fillPath(path, tool->color());
            break;
        }
        case Magnifier:
        {
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setPen(tool->color());
            painter->drawEllipse(tool->pos(), tool->size(), tool->size());
            showMagnifier(painter, tool);
            break;
        }
        default:
            break;
        }
    }
}
