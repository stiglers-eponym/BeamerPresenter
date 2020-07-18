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
    //qDebug() << event;
    switch (event->type())
    {
    case QEvent::GraphicsSceneMouseRelease:
    {
        const auto mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        const PdfLink link = master->resolveLink(preferences().page + shift & ~AnyOverlay, mouseevent->scenePos());
        if (link.type >= 0)
            emit navigationEvent(link.type);
    }
    }
    return false;
}

void SlideScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // TODO!
    // This doesn't do anything!
}

void SlideScene::receiveAction(const Action action)
{
    switch (action)
    {
    case Update:
        navigationEvent(preferences().page);
        break;
    default:
        break;
    }
}

void SlideScene::navigationEvent(const int page)
{
    int newpage;
    if (shift & ShiftOverlays::AnyOverlay)
        newpage = master->overlaysShifted(page, shift);
    else
        newpage = page + shift;
    const QSizeF pagesize = master->getPageSize(newpage);
    setSceneRect(0., 0., pagesize.width(), pagesize.height());
    emit navigationToViews(newpage, pagesize);
    invalidate();
}
