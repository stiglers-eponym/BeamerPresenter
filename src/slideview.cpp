#include "src/slideview.h"
#include "src/rendering/pixcache.h"
#include "src/slidescene.h"
#include "src/preferences.h"
#include "src/pdfmaster.h"

SlideView::SlideView(SlideScene *scene, PixCache *cache, QWidget *parent) :
    QGraphicsView(scene, parent),
    GuiWidget(WidgetType::Slide)
{
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
    QPalette newpalette = palette();
    newpalette.setColor(QPalette::Base, Qt::black);
#if QT_VERSION_MAJOR <= 5
    newpalette.setColor(QPalette::Background, Qt::black);
#endif
    setPalette(newpalette);
}

void SlideView::pageChanged(const int page, const QSizeF &pageSize, SlideScene *scene)
{
    setScene(scene);
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
    qDebug() << "Request page" << page << this;
    emit requestPage(page, resolution);
}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // only for testing:
    painter->fillRect(rect, Qt::green);

    painter->drawPixmap(scene()->sceneRect(), currentPixmap, currentPixmap.rect());
}

void SlideView::pageReady(const QPixmap pixmap, const int page)
{
    if (waitingForPage != page)
        return;
    currentPixmap = pixmap;
    waitingForPage = -1;
    updateScene({sceneRect()});
}

void SlideView::resizeEvent(QResizeEvent *event)
{
    emit resizeCache(event->size());
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
    emit sendKeyEvent(event);
}

const QSizeF SlideView::preferredSize(const QSizeF &parent_size) const
{
    if (preferred_size.isEmpty())
        return preferred_size;
    QSizeF boundary = parent_size;
    if (preferred_size.width() > 0.)
        boundary.rwidth() *= preferred_size.width();
    if (preferred_size.height() > 0.)
        boundary.rheight() *= preferred_size.height();
    const QSizeF reference = scene()->sceneRect().size();
    const qreal aspect = reference.width() / reference.height();
    if (aspect * boundary.height() > boundary.width())
        // page is wider than available geometry.
        boundary.setHeight(boundary.width()/aspect);
    else
        // page is higher than available geometry.
        boundary.setWidth(boundary.height()*aspect);
    qDebug() << parent_size << boundary << this;
    return boundary;
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
        event->setAccepted(true);
        return true;
    }
    case QEvent::TabletRelease:
    {
        auto tabletevent = static_cast<QTabletEvent*>(event);
        static_cast<SlideScene*>(scene())->tabletRelease(mapToScene(tabletevent->posF()), tabletevent);
        event->setAccepted(true);
        return true;
    }
    case QEvent::TabletMove:
    {
        auto tabletevent = static_cast<QTabletEvent*>(event);
        static_cast<SlideScene*>(scene())->tabletMove(mapToScene(tabletevent->posF()), tabletevent);
        event->setAccepted(true);
        return true;
    }
    default:
        event->setAccepted(false);
        return QGraphicsView::event(event);
    }
}
