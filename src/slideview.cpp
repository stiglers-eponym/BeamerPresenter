#include "src/slideview.h"
#include "src/rendering/pixcache.h"

SlideView::SlideView(QGraphicsScene *scene, PixCache *pixcache, QWidget *parent) :
    QGraphicsView(scene, parent),
    GuiWidget(WidgetType::Slide),
    pixcache(pixcache)
{

}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // TODO!
    painter->drawPixmap(0, 0, currentPixmap);
}

void SlideView::pageChanged(const int page, const QSizeF &pagesize)
{
    // TODO: handle pagesize
    currentPixmap = *pixcache->pixmap(page);
    update();
}
