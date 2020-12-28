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
    for (const auto item : items())
        removeItem(item);
    delete currentPath;
    delete currentItemCollection;
}

void SlideScene::stopDrawing()
{
    qDebug() << "Stop drawing" << page << page_part;
    if (currentPath && currentPath->size() > 1)
    {
        addItem(currentPath);
        currentPath->show();
        emit sendNewPath(page | page_part, currentPath);
        invalidate(currentPath->boundingRect());
        update(currentPath->boundingRect());
    }
    currentPath = nullptr;
    if (currentItemCollection)
    {
        removeItem(currentItemCollection);
        delete currentItemCollection;
        currentItemCollection = nullptr;
    }
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
    // TODO
}

void SlideScene::receiveAction(const Action action)
{
    // TODO: necessary?
    switch (action)
    {
    default:
        break;
    }
}

void SlideScene::navigationEvent(const int newpage, SlideScene *newscene)
{
    /// Page size in points.
    QSizeF pagesize = master->getPageSize(newpage);
    switch (page_part)
    {
    case LeftHalf:
        pagesize.rwidth() /= 2;
        setSceneRect(0., 0., pagesize.width(), pagesize.height());
        break;
    case RightHalf:
        pagesize.rwidth() /= 2;
        setSceneRect(pagesize.width(), 0., pagesize.width(), pagesize.height());
        break;
    default:
        setSceneRect(0., 0., pagesize.width(), pagesize.height());
        break;
    }
    if (show_animations && (!newscene || newscene == this))
    {
        const SlideTransition transition = master->transition(newpage);
        if (transition.type)
        {
            // TODO!
            qDebug() << "Transition:" << transition.type << transition.duration << transition.properties << transition.angle << transition.scale;
            startTransition(newpage, transition);
            return;
        }
    }
    page = newpage;
    emit navigationToViews(page, pagesize, newscene ? newscene : this);
    for (const auto item : items())
        removeItem(item);
    if (!newscene || newscene == this)
    {
        const auto paths = master->pathContainer(page | page_part);
        if (paths)
        {
            const auto end = paths->cend();
            for (auto it = paths->cbegin(); it != end; ++it)
                addItem(*it);
        }
    }
    invalidate();
}

void SlideScene::startTransition(const int newpage, const SlideTransition &transition)
{
    // TODO!
    page = newpage;
    emit navigationToViews(page, sceneRect().size(), this);
    for (const auto item : items())
        removeItem(item);
    const auto paths = master->pathContainer(page | page_part);
    if (paths)
    {
        const auto end = paths->cend();
        for (auto it = paths->cbegin(); it != end; ++it)
            addItem(*it);
    }
    invalidate();
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    if (event->pressure() <= 0)
        return;
    switch (event->pointerType())
    {
    case QTabletEvent::Pen:
        if (currentPath && currentItemCollection)
        {
            auto item = new QGraphicsLineItem(QLineF(currentPath->lastPoint(), pos));
            item->setPen(QPen(QBrush(Qt::red), event->pressure(), Qt::SolidLine, Qt::RoundCap));
            item->show();
            addItem(item);
            currentItemCollection->addToGroup(item);
            currentItemCollection->show();
            update(item->boundingRect());
            invalidate(item->boundingRect());
            static_cast<FullGraphicsPath*>(currentPath)->addPoint(pos, event->pressure());
        }
        break;
    case QTabletEvent::Eraser:
    {
        auto container = master->pathContainer(page | page_part);
        if (container)
            container->eraserMicroStep(pos);
        break;
    }
    default:
        break;
    }
}

void SlideScene::tabletPress(const QPointF &pos, const QTabletEvent *event)
{
    switch (event->pointerType())
    {
    case QTabletEvent::Pen:
    {
        currentItemCollection = new QGraphicsItemGroup();
        addItem(currentItemCollection);
        currentItemCollection->show();
        currentPath = new FullGraphicsPath(pos, event->pressure());
        break;
    }
    case QTabletEvent::Eraser:
    {
        auto container = master->pathContainer(page | page_part);
        if (container)
            container->startMicroStep();
        break;
    }
    default:
        break;
    }
}

void SlideScene::tabletRelease(const QPointF &pos, const QTabletEvent *event)
{
    stopDrawing();
    switch (event->pointerType())
    {
    case QTabletEvent::Pen:
        invalidate({sceneRect()});
        update({sceneRect()});
        break;
    case QTabletEvent::Eraser:
    {
        auto container = master->pathContainer(page | page_part);
        if (container)
            container->applyMicroStep();
        break;
    }
    default:
        break;
    }
}
