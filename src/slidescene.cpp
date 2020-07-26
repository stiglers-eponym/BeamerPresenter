#include "src/slidescene.h"
#include "src/pdfmaster.h"

SlideScene::SlideScene(const PdfMaster *master, const PagePart part, QObject *parent) :
    QGraphicsScene(parent),
    master(master),
    page_part(part)
{}

void SlideScene::stopDrawing()
{
    if (currentPath.isEmpty())
        return;
    // TODO: add path to other paths.
}

unsigned int SlideScene::identifier() const
{
    return qHash(QPair<int, const void*>(shift, master)) + page_part;
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
        master->resolveLink(preferences().page + shift & ~AnyOverlay, mouseevent->scenePos());
        return true;
    }
    default:
        return false;
    }
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
    QSizeF pagesize = master->getPageSize(newpage);
    switch (page_part)
    {
    case FullPage:
        setSceneRect(0., 0., pagesize.width(), pagesize.height());
        break;
    case LeftHalf:
        pagesize.rwidth() /= 2;
        setSceneRect(0., 0., pagesize.width(), pagesize.height());
        break;
    case RightHalf:
        pagesize.rwidth() /= 2;
        setSceneRect(pagesize.width(), 0., pagesize.width(), pagesize.height());
        break;
    }
    emit navigationToViews(newpage, pagesize);
    invalidate();
}
