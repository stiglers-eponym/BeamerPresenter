#include "src/slideview.h"
#include "src/rendering/pixcache.h"
#include "src/slidescene.h"

SlideView::SlideView(SlideScene *scene, PixCache *cache, QWidget *parent) :
    QGraphicsView(scene, parent),
    GuiWidget(WidgetType::Slide),
    pixcache(cache)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFocusPolicy(Qt::StrongFocus);
    if (pixcache == nullptr)
        pixcache = new PixCache(scene->getPdfMaster(), 1, this);
    pixcache->updateFrame(size());
}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // TODO!
    painter->drawPixmap(0, 0, currentPixmap);
}

void SlideView::pageChanged(const int page, const QSizeF &pagesize)
{
    // TODO: handle pagesize
    currentPixmap = pixcache->pixmap(page);
    update();
}

void SlideView::pageReady(const int page, const qreal resolution)
{
    // TODO
}

void SlideView::resizeEvent(QResizeEvent *event)
{
    pixcache->updateFrame(size());
    pixcache->startRendering();
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
    emit sendKeyEvent(event);
}
