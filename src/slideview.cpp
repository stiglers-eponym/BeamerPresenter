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
    QRectF bgrect = scene()->sceneRect();
    bgrect.moveTop(0);
    painter->drawPixmap(bgrect, currentPixmap, currentPixmap.rect());
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
    if (static_cast<const SlideScene*>(scene())->isTextEditing() && !(event->modifiers() & Qt::CTRL))
    {
        if (event->key() == Qt::Key_Escape)
            scene()->clearFocus();
        else
            QGraphicsView::keyPressEvent(event);
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

void SlideView::showMagnifier(QPainter *painter, const PointingTool *tool)
{
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);
    painter->setPen(tool->color());
    if (enlargedPixmap.isNull() && waitingForPage == INT_MAX)
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
    for (const auto &pos : tool->pos())
    {
        const QRectF scene_rect(pos.x()-tool->size(), pos.y()-tool->size(), 2*tool->size(), 2*tool->size());
        QPainterPath path;
        path.addEllipse(scene_rect);
        painter->setClipPath(path);
        painter->fillPath(path, QBrush(palette().base()));
        if (path.intersects(sceneRect()))
        {
            if (enlargedPixmap.isNull())
            {
                const qreal scale = currentPixmap.width() / sceneRect().width();
                QRectF pixmap_rect(pos.x()-tool->size()/tool->scale(), pos.y()-tool->size()/tool->scale(), tool->size()*2/tool->scale(), tool->size()*2/tool->scale());
                pixmap_rect.setRect(scale*pixmap_rect.x(), scale*pixmap_rect.y(), scale*pixmap_rect.width(), scale*pixmap_rect.height());
                painter->drawPixmap(scene_rect, currentPixmap, pixmap_rect);
            }
            else
            {
                const qreal scale = enlargedPixmap.width() / sceneRect().width();
                QRectF pixmap_rect(pos.x()-tool->size()/tool->scale(), pos.y()-tool->size()/tool->scale(), tool->size()*2/tool->scale(), tool->size()*2/tool->scale());
                pixmap_rect.setRect(scale*pixmap_rect.x(), scale*pixmap_rect.y(), scale*pixmap_rect.width(), scale*pixmap_rect.height());
                painter->drawPixmap(scene_rect, enlargedPixmap, pixmap_rect);
            }
            // Draw paths. But don't do that while something is being drawn.
            // That could lead to a segmentation fault.
            if (!static_cast<SlideScene*>(scene())->isDrawing())
            {
                painter->save();
                //painter->setTransform(QTransform::fromScale(tool->scale(),tool->scale()).translate(-tool->pos().x()*(1-1/tool->scale()), -tool->pos().y()*(1-1/tool->scale())), true);
                painter->setTransform(QTransform::fromTranslate(pos.x()*(1-tool->scale()), pos.y()*(1-tool->scale())).scale(tool->scale(),tool->scale()), true);
                for (const auto item : static_cast<const QList<QGraphicsItem*>>(items()))
                    item->paint(painter, NULL, this);
                painter->restore();
            }
        }
        painter->drawEllipse(pos, tool->size(), tool->size());
    }
}

void SlideView::drawForeground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing);
    for (const auto basic_tool : preferences()->current_tools)
    {
        if (!(basic_tool->tool() & AnyPointingTool))
            continue;
        const PointingTool *tool = static_cast<PointingTool*>(basic_tool);
        if (tool->pos().isEmpty())
            continue;
        debug_verbose(DebugDrawing) << "drawing tool" << tool->tool() << tool->size() << tool->color();
        switch (tool->tool())
        {
        case Pointer:
        {
            painter->setPen(Qt::PenStyle::NoPen);
            painter->setBrush(tool->brush());
            painter->setCompositionMode(QPainter::CompositionMode_Darken);
            for (const auto &pos : tool->pos())
                painter->drawEllipse(pos, tool->size(), tool->size());
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            for (const auto &pos : tool->pos())
                painter->drawEllipse(pos, tool->size(), tool->size());
            break;
        }
        case Torch:
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
            break;
        }
        case Magnifier:
        {
            showMagnifier(painter, tool);
            break;
        }
        default:
            break;
        }
    }
}
