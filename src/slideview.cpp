#include "src/slideview.h"

SlideView::SlideView(QWidget *parent) : QGraphicsView(parent)
{

}

void SlideView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // TODO!
    painter->drawPixmap(0, 0, currentPixmap);
}
