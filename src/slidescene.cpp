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
        for (const auto tool : preferences().current_tools)
        {
            if (tool && (tool->device() & mouse_to_input_device.value(mouseevent->buttons())))
            {
                startInputEvent(tool, mouseevent->scenePos());
                return true;
            }
        }
        return false;
    }
    case QEvent::GraphicsSceneMouseMove:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (current_tool && (current_tool->device() & mouse_to_input_device.value(mouseevent->buttons())))
        {
            stepInputEvent(mouseevent->scenePos());
            return true;
        }
        return false;
    }
    case QEvent::GraphicsSceneMouseRelease:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        for (const auto tool : preferences().current_tools)
        {
            if (tool && (tool->device() & mouse_to_input_device.value(mouseevent->button())))
            {
                stopInputEvent();
                return true;
            }
        }
        return false;
    }
    /*
    case QEvent::TouchUpdate:
    {
        const auto touchevent = static_cast<QTouchEvent*>(event);
        qDebug() << touchevent;
        return false;
    }
    */
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

void SlideScene::tabletPress(const QPointF &pos, const QTabletEvent *event)
{
    for (const auto tool : preferences().current_tools)
    {
        if (tool && (tool->device() & tablet_device_to_input_device.value(event->pointerType())))
        {
            startInputEvent(tool, pos, event->pressure());
            return;
        }
    }
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    if (current_tool && event->pressure() > 0 && (current_tool->device() & tablet_device_to_input_device.value(event->pointerType())))
        stepInputEvent(pos, event->pressure());
}

void SlideScene::tabletRelease(const QTabletEvent *event)
{
    for (const auto tool : preferences().current_tools)
    {
        if (tool && (tool->device() & tablet_device_to_input_device.value(event->pointerType())))
        {
            stopInputEvent();
            return;
        }
    }
}

void SlideScene::startInputEvent(const Tool *tool, const QPointF &pos, const float pressure)
{
    if (!tool)
        return;
    //qDebug() << "Start input event" << tool->tool() << tool->device() << tool << pressure;
    stopDrawing();
    if (current_tool)
        qWarning() << "Start drawing, but last drawing event was not properly completed.";
    current_tool = tool;
    switch (tool->tool())
    {
    case Pen:
    case Highlighter:
        if (currentItemCollection || currentPath)
            break;
        currentItemCollection = new QGraphicsItemGroup();
        addItem(currentItemCollection);
        currentItemCollection->show();
        if (tool->tool() == Pen && (tool->device() & PressureSensitiveDevice))
            currentPath = new FullGraphicsPath(*static_cast<const DrawTool*>(tool), pos, pressure);
        else
            currentPath = new BasicGraphicsPath(*static_cast<const DrawTool*>(tool), pos);
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

void SlideScene::stepInputEvent(const QPointF &pos, const float pressure)
{
    if (pressure <= 0 || !current_tool)
        return;
    //qDebug() << "Step input event" << current_tool->tool() << current_tool->device() << current_tool << pressure;
    switch (current_tool->tool())
    {
    case Pen:
    case Highlighter:
        if (currentPath && currentItemCollection && *static_cast<const DrawTool*>(current_tool) == currentPath->getTool())
        {
            auto item = new FlexGraphicsLineItem(QLineF(currentPath->lastPoint(), pos), currentPath->getTool().compositionMode());
            if (currentPath->type() == QGraphicsPathItem::UserType + 2)
            {
                static_cast<FullGraphicsPath*>(currentPath)->addPoint(pos, pressure);
                QPen pen = currentPath->getTool().pen();
                pen.setWidthF(pen.widthF() * pressure);
                item->setPen(pen);
            }
            else if (currentPath->type() == QGraphicsPathItem::UserType + 1)
            {
                static_cast<BasicGraphicsPath*>(currentPath)->addPoint(pos);
                item->setPen(currentPath->getTool().pen());
            }
            else
                qCritical() << "This should never happen.";
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

void SlideScene::stopInputEvent()
{
    if (!current_tool)
        return;
    //qDebug() << "Stop input event" << current_tool->tool() << current_tool->device() << current_tool;
    stopDrawing();
    switch (current_tool->tool())
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
    current_tool = nullptr;
}
