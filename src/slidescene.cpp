#include "src/slidescene.h"
#include "src/pdfmaster.h"

SlideScene::SlideScene(const PdfMaster *master, const PagePart part, QObject *parent) :
    QGraphicsScene(parent),
    master(master),
    page_part(part)
{
    connect(this, &SlideScene::sendNewPath, master, &PdfMaster::receiveNewPath);
}

SlideScene::~SlideScene()
{
    delete currentPath;
    clear();
}

void SlideScene::stopDrawing()
{
    if (currentPath && currentPath->size() > 1)
    {
        addItem(currentPath);
        currentPath->show();
        emit sendNewPath(page, currentPath);
        invalidate(currentPath->boundingRect());
        update(currentPath->boundingRect());
    }
    currentPath = nullptr;
    removeItem(currentItemCollection);
    delete currentItemCollection;
    currentItemCollection = nullptr;
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
        qDebug() << mouseevent;
        master->resolveLink(page, mouseevent->scenePos());
        return true;
    }
    case QEvent::TouchUpdate:
    {
        const auto touchevent = static_cast<QTouchEvent*>(event);
        qDebug() << touchevent;
    }
    case QEvent::GraphicsSceneMouseMove:
    {
        const auto mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        qDebug() << mouseevent;
    }
    default:
        event->setAccepted(false);
        return QGraphicsScene::event(event);
    }
}

void SlideScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // TODO!
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

void SlideScene::navigationEvent(const int newpage)
{
    if (shift & ShiftOverlays::AnyOverlay)
        page = master->overlaysShifted(newpage, shift);
    else
        page = newpage + shift;
    QSizeF pagesize = master->getPageSize(page);
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
    emit navigationToViews(page, pagesize);
    invalidate();
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    if (event->pressure() > 0 && currentPath && currentItemCollection)
    {
        auto item = new QGraphicsLineItem(QLineF(currentPath->lastPoint(), pos));
        item->setPen(QPen(QBrush(Qt::red), 10*event->pressure(), Qt::SolidLine, Qt::RoundCap));
        item->show();
        addItem(item);
        currentItemCollection->addToGroup(item);
        currentItemCollection->show();
        update(item->boundingRect());
        invalidate(item->boundingRect());
        static_cast<FullGraphicsPath*>(currentPath)->addPoint(pos, event->pressure());
    }
}

void SlideScene::tabletPress(const QPointF &pos, const QTabletEvent *event)
{
    qDebug() << pos << event->pressure();
    currentItemCollection = new QGraphicsItemGroup();
    addItem(currentItemCollection);
    currentItemCollection->show();
    currentPath = new FullGraphicsPath();
    static_cast<FullGraphicsPath*>(currentPath)->addPoint(pos, event->pressure());
}

void SlideScene::tabletRelease(const QPointF &pos, const QTabletEvent *event)
{
    qDebug() << pos;
    stopDrawing();
    invalidate({sceneRect()});
    update({sceneRect()});
}
