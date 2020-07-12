#include "src/slideview.h"
#include "src/rendering/pixcache.h"
#include "src/slidescene.h"
#include "src/preferences.h"

SlideView::SlideView(SlideScene *scene, PixCache *cache, QWidget *parent) :
    QGraphicsView(scene, parent),
    GuiWidget(WidgetType::Slide),
    pixcache(cache)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFocusPolicy(Qt::StrongFocus);
    setFrameShape(QFrame::NoFrame);
    if (pixcache == nullptr)
        pixcache = new PixCache(scene->getPdfMaster(), 1, this);
    pixcache->updateFrame(size());
    QPalette newpalette = palette();
    newpalette.setColor(QPalette::Base, Qt::black);
    newpalette.setColor(QPalette::Background, Qt::black);
    setPalette(newpalette);
}

void SlideView::pageChanged(const int page, const QSizeF &pageSize)
{
    // TODO: handle pagesize
    currentPixmap = pixcache->pixmap(page);
    qreal resolution;
    if (pageSize.width() * height() > pageSize.height() * width())
        // page is too wide, determine resolution by x direction
        resolution = width() / pageSize.width();
    else
        // page is too high, determine resolution by y direction
        resolution = height() / pageSize.height();
    resetTransform();
    scale(resolution, resolution);
}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawPixmap(scene()->sceneRect(), currentPixmap, currentPixmap.rect());
}

void SlideView::pageReady(const int page, const qreal resolution)
{
    // TODO
}

void SlideView::resizeEvent(QResizeEvent *event)
{
    pixcache->updateFrame(size());
    pixcache->clear();
    pixcache->startRendering();
}

void SlideView::keyPressEvent(QKeyEvent *event)
{
    emit sendKeyEvent(event);
}
