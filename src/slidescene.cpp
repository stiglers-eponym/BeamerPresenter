#include "src/slidescene.h"
#include "src/pdfmaster.h"

SlideScene::SlideScene(const PdfMaster *master, QObject *parent) :
    QGraphicsScene(parent),
    master(master)
{

}

void SlideScene::stopDrawing()
{
    if (currentPath.isEmpty())
        return;
    // TODO: add path to other paths.
}

unsigned int SlideScene::identifier() const
{
    return qHash(QPair<int, const void*>(shift, master));
}

bool SlideScene::event(QEvent* event)
{
    // TODO!
    return false;
}

void SlideScene::receiveAction(const Action action)
{
    switch (action)
    {
    case Update:
        navigationEvent(preferences().page);
        update();
        break;
    default:
        break;
    }
}

void SlideScene::navigationEvent(const int page)
{
    int newpage = page + (shift & ~ShiftOverlays::AnyOverlay);
    if (shift & ShiftOverlays::AnyOverlay)
        newpage = master->overlaysShifted(page, shift);
    const QSizeF pagesize = master->getPageSize(newpage);
    setSceneRect(0., 0., pagesize.width(), pagesize.height());
    emit navigationToViews(newpage, pagesize);
    invalidate();
}
