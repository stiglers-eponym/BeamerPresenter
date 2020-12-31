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
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    delete currentPath;
    delete currentItemCollection;
}

void SlideScene::stopDrawing()
{
    qDebug() << "Stop drawing" << page << page_part;
    if (currentPath && currentPath->size() > 1)
    {
        currentPath->show();
        emit sendNewPath(page | page_part, currentPath);
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

bool SlideScene::event(QEvent* event)
{
    // TODO!
    //qDebug() << event;
    switch (event->type())
    {
    case QEvent::GraphicsSceneMousePress:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (mouseevent->buttons() == Qt::LeftButton && preferences().current_tool)
        {
            stopDrawing();
            switch (preferences().current_tool->tool())
            {
            case Pen:
            case Highlighter:
                if (currentItemCollection || currentPath)
                    return false;
                currentItemCollection = new QGraphicsItemGroup();
                addItem(currentItemCollection);
                currentPath = new BasicGraphicsPath(*static_cast<DrawTool*>(preferences().current_tool), mouseevent->scenePos());
                addItem(currentPath);
                currentPath->hide();
                currentItemCollection->show();
                return true;
            case Eraser:
            {
                auto container = master->pathContainer(page | page_part);
                if (container)
                    container->startMicroStep();
                return true;
            }
            default:
                return false;
            }
        }
        else if (mouseevent->buttons() == Qt::RightButton)
        {
            auto container = master->pathContainer(page | page_part);
            if (container)
                container->startMicroStep();
            return true;
        }
        else
            return false;
    }
    case QEvent::GraphicsSceneMouseRelease:
    {
        stopDrawing();
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (mouseevent->button() == Qt::LeftButton)
        {
            switch (preferences().current_tool ? preferences().current_tool->tool() : NoTool)
            {
            case Pen:
            case Highlighter:
                if (currentPath)
                {
                    invalidate({sceneRect()});
                    update({sceneRect()});
                    return true;
                }
                return false;
            case Eraser:
            {
                auto container = master->pathContainer(page | page_part);
                if (container)
                    container->applyMicroStep();
                return true;
            }
            case NoTool:
            case InvalidTool:
                master->resolveLink(page, mouseevent->scenePos());
                return true;
            default:
                return false;
            }
        }
        else if (mouseevent->button() == Qt::RightButton)
        {
            auto container = master->pathContainer(page | page_part);
            if (container)
                container->applyMicroStep();
            return true;
        }
        else
            return false;
    }
    case QEvent::TouchUpdate:
    {
        const auto touchevent = static_cast<QTouchEvent*>(event);
        return false;
    }
    case QEvent::GraphicsSceneMouseMove:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (mouseevent->buttons() == Qt::LeftButton && preferences().current_tool)
        {
            switch (preferences().current_tool->tool())
            {
            case Pen:
            case Highlighter:
                if (currentPath && currentItemCollection)
                {
                    auto item = new FlexGraphicsLineItem(QLineF(currentPath->lastPoint(), mouseevent->scenePos()), currentPath->getTool().compositionMode());
                    static_cast<BasicGraphicsPath*>(currentPath)->addPoint(mouseevent->scenePos());
                    item->setPen(currentPath->getTool().pen());
                    item->show();
                    addItem(item);
                    currentItemCollection->addToGroup(item);
                    currentItemCollection->show();
                    update(item->boundingRect());
                    invalidate(item->boundingRect());
                }
                return true;
            case Eraser:
            {
                auto container = master->pathContainer(page | page_part);
                if (container)
                    container->eraserMicroStep(mouseevent->scenePos());
                return true;
            }
            default:
                return false;
            }
        }
        else if (mouseevent->buttons() == Qt::RightButton)
        {
            auto container = master->pathContainer(page | page_part);
            if (container)
                container->eraserMicroStep(mouseevent->scenePos());
            return true;
        }
        else
            return false;
    }
    default:
        event->setAccepted(false);
        return QGraphicsScene::event(event);
    }
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
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
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
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    const auto paths = master->pathContainer(page | page_part);
    if (paths)
    {
        const auto end = paths->cend();
        for (auto it = paths->cbegin(); it != end; ++it)
            if (*it)
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
        if (preferences().current_tablet_tool)
        {
            switch (preferences().current_tablet_tool->tool())
            {
            case Pen:
                if (currentPath && currentItemCollection)
                {
                    auto item = new FlexGraphicsLineItem(QLineF(currentPath->lastPoint(), pos), currentPath->getTool().compositionMode());
                    static_cast<FullGraphicsPath*>(currentPath)->addPoint(pos, event->pressure());
                    QPen pen = currentPath->getTool().pen();
                    pen.setWidthF(pen.widthF() * event->pressure());
                    item->setPen(pen);
                    item->show();
                    addItem(item);
                    currentItemCollection->addToGroup(item);
                    currentItemCollection->show();
                    update(item->boundingRect());
                    invalidate(item->boundingRect());
                }
                break;
            case Highlighter:
                if (currentPath && currentItemCollection)
                {
                    auto item = new FlexGraphicsLineItem(QLineF(currentPath->lastPoint(), pos), currentPath->getTool().compositionMode());
                    static_cast<BasicGraphicsPath*>(currentPath)->addPoint(pos);
                    item->setPen(currentPath->getTool().pen());
                    item->show();
                    addItem(item);
                    currentItemCollection->addToGroup(item);
                    currentItemCollection->show();
                    update(item->boundingRect());
                    invalidate(item->boundingRect());
                }
                break;
            case Eraser:
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
    stopDrawing();
    switch (event->pointerType())
    {
    case QTabletEvent::Pen:
    {
        if (preferences().current_tablet_tool)
        {
            switch (preferences().current_tablet_tool->tool())
            {
            case Pen:
                if (currentItemCollection || currentPath)
                    break;
                currentItemCollection = new QGraphicsItemGroup();
                addItem(currentItemCollection);
                currentItemCollection->show();
                currentPath = new FullGraphicsPath(*static_cast<DrawTool*>(preferences().current_tablet_tool), pos, event->pressure());
                addItem(currentPath);
                currentPath->hide();
                break;
            case Highlighter:
                if (currentItemCollection || currentPath)
                    break;
                currentItemCollection = new QGraphicsItemGroup();
                addItem(currentItemCollection);
                currentItemCollection->show();
                currentPath = new BasicGraphicsPath(*static_cast<DrawTool*>(preferences().current_tablet_tool), pos);
                addItem(currentPath);
                currentPath->hide();
                break;
            case Eraser:
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
        if (preferences().current_tablet_tool)
        {
            switch (preferences().current_tablet_tool->tool())
            {
            case Pen:
            case Highlighter:
                invalidate({sceneRect()});
                update({sceneRect()});
                break;
            case Eraser:
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
