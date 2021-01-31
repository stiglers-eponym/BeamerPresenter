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
    debug_msg(DebugDrawing) << "Stop drawing" << page << page_part;
    if (currentPath && currentPath->size() > 1)
    {
        currentPath->show();
        emit sendNewPath(page | page_part, currentPath);
        invalidate(currentPath->boundingRect(), QGraphicsScene::ItemLayer);
    }
    currentPath = NULL;
    if (currentItemCollection)
    {
        removeItem(currentItemCollection);
        delete currentItemCollection;
        currentItemCollection = NULL;
    }
}

bool SlideScene::event(QEvent* event)
{
    debug_verbose(DebugDrawing) << event;
    switch (event->type())
    {
    case QEvent::GraphicsSceneMousePress:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        Tool *const tool = preferences()->currentTool(mouseevent->buttons() << 1);
        if (!tool)
            return false;
        if (tool->tool() & BasicTool::AnyDrawTool)
        {
            startInputEvent(tool, mouseevent->scenePos());
            event->accept();
            return true;
        }
        else if (tool->tool() & AnyPointingTool)
        {
            static_cast<PointingTool*>(tool)->setPos(mouseevent->scenePos());
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            event->accept();
            return true;
        }
        return false;
    }
    case QEvent::GraphicsSceneMouseMove:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        const int device = mouseevent->buttons() ? mouseevent->buttons() << 1 : 1;
        if (current_tool && (current_tool->device() & device))
        {
            stepInputEvent(mouseevent->scenePos());
            event->accept();
            return true;
        }
        else {
            Tool *tool = preferences()->currentTool(device);
            if (tool && (tool->tool() & AnyPointingTool))
            {
                const QPointF oldpos = static_cast<PointingTool*>(tool)->pos();
                const QPointF &newpos = mouseevent->scenePos();
                static_cast<PointingTool*>(tool)->setPos(newpos);
                invalidate(QRectF(oldpos, newpos).normalized().marginsAdded(static_cast<PointingTool*>(tool)->size() * QMarginsF(1,1,1,1)), QGraphicsScene::ForegroundLayer);
                event->accept();
                return true;
            }
        }
        return false;
    }
    case QEvent::GraphicsSceneMouseRelease:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        if (current_tool)
            stopInputEvent(mouseevent->scenePos());
        const int device = mouseevent->button() << 1;
        Tool *tool = preferences()->currentTool(device);
        if (tool && (tool->tool() & AnyPointingTool) && !(tool->device() & MouseNoButton))
        {
            static_cast<PointingTool*>(tool)->setPos({0,0});
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
        }
        event->accept();
        return true;
    }
    case QEvent::TouchBegin:
    {
        Tool *const tool = preferences()->currentTool(TouchInput);
        if (!tool)
            return false;
        const auto touchevent = static_cast<QTouchEvent*>(event);
        if ((tool->tool() & AnyDrawTool) && (touchevent->touchPoints().size() == 1))
        {
            const QTouchEvent::TouchPoint &point = touchevent->touchPoints().first();
            startInputEvent(tool, point.scenePos(), point.pressure());
            event->accept();
            return true;
        }
        if (tool->tool() & AnyPointingTool)
        {
            if (touchevent->touchPoints().isEmpty())
                return true;
            static_cast<PointingTool*>(tool)->setPos(touchevent->touchPoints().last().scenePos());
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            event->accept();
            return true;
        }
        return false;
    }
    case QEvent::TouchUpdate:
    {
        const auto touchevent = static_cast<QTouchEvent*>(event);
        if (current_tool && (current_tool->device() & TouchInput))
        {
            if (touchevent->touchPoints().size() == 1)
            {
                const QTouchEvent::TouchPoint &point = touchevent->touchPoints().first();
                stepInputEvent(point.scenePos(), point.pressure());
            }
            // Touching with a second finger stops the drawing.
            else
                stopInputEvent(QPointF());
            // Alternatively, touching with a second finger could cancel
            // drawing and revert the already drawn path:
            //else if (stopInputEvent(QPointF()))
            //    master->pathContainer(page | page_part)->undo(this);
            event->accept();
            return true;
        }
        else {
            Tool *tool = preferences()->currentTool(TouchInput);
            if (tool && (tool->tool() & AnyPointingTool))
            {
                const QPointF oldpos = static_cast<PointingTool*>(tool)->pos();
                const QPointF &newpos = touchevent->touchPoints().last().scenePos();
                static_cast<PointingTool*>(tool)->setPos(newpos);
                invalidate(QRectF(oldpos, newpos).normalized().marginsAdded(static_cast<PointingTool*>(tool)->size() * QMarginsF(1,1,1,1)), QGraphicsScene::ForegroundLayer);
                event->accept();
                return true;
            }
        }
        return false;
    }
    case QEvent::TouchCancel:
        if (current_tool && (current_tool->device() & TouchInput))
        {
            if (stopInputEvent(QPointF()))
                master->pathContainer(page | page_part)->undo(this);
            Tool *tool = preferences()->currentTool(TouchInput);
            if (tool && (tool->tool() & AnyPointingTool))
            {
                static_cast<PointingTool*>(tool)->setPos({0,0});
                invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            }
            event->accept();
            return true;
        }
        return false;
    case QEvent::TouchEnd:
        {
            const auto touchevent = static_cast<QTouchEvent*>(event);
            if (current_tool && (current_tool->device() & TouchInput))
            {
                if (touchevent->touchPoints().size() == 1)
                    stopInputEvent(touchevent->touchPoints().first().scenePos());
                else
                    stopInputEvent(QPointF());
            }
            Tool *tool = preferences()->currentTool(TouchInput);
            if (tool && (tool->tool() & AnyPointingTool))
            {
                static_cast<PointingTool*>(tool)->setPos({0,0});
                invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            }
            event->accept();
            return true;
        }
    default:
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

void SlideScene::prepareNavigationEvent(const int newpage)
{
    // Adjust scene size.
    /// Page size in points.
    QSizeF pagesize = master->getPageSize(master->overlaysShifted(newpage, shift));
    debug_verbose(DebugPageChange) << newpage << pagesize << master->getDocument()->flexiblePageSizes();
    // Don't do anything if page size ist not valid. This avoids cleared slide
    // scenes which could mess up the layout and invalidate cache.
    if ((pagesize.isNull() || !pagesize.isValid()) && !master->getDocument()->flexiblePageSizes())
    {
        emit clearViews();
        return;
    }
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
}

void SlideScene::navigationEvent(const int newpage, SlideScene *newscene)
{
    if (show_animations && (!newscene || newscene == this))
    {
        const SlideTransition transition = master->transition(newpage);
        if (transition.type)
        {
            // TODO!
            debug_msg(DebugTransitions) << "Transition:" << transition.type << transition.duration << transition.properties << transition.angle << transition.scale;
            startTransition(newpage, transition);
            return;
        }
    }
    page = newpage;
    emit navigationToViews(page, newscene ? newscene : this);
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
    emit navigationToViews(page, this);
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
    Tool *tool = preferences()->currentTool(
                event->pressure() > 0 ?
                tablet_device_to_input_device.value(event->pointerType()) :
                TabletNoPressure
            );
    if (tool && (tool->tool() & AnyDrawTool))
    {
        startInputEvent(tool, pos, event->pressure());
        return;
    }
    else if (tool && (tool->tool() & AnyPointingTool))
    {
        static_cast<PointingTool*>(tool)->setPos(pos);
        invalidate(QRect(), QGraphicsScene::ForegroundLayer);
    }
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    if (current_tool && event->pressure() > 0 && (current_tool->device() & tablet_device_to_input_device.value(event->pointerType())))
        stepInputEvent(pos, event->pressure());
    else
    {
        Tool *tool = preferences()->currentTool(
                    event->pressure() > 0 ?
                    tablet_device_to_input_device.value(event->pointerType()) :
                    TabletNoPressure
                );
        if (tool && (tool->tool() & AnyPointingTool))
        {
            const QPointF oldpos = static_cast<PointingTool*>(tool)->pos();
            static_cast<PointingTool*>(tool)->setPos(pos);
            invalidate(QRectF(oldpos, pos).normalized().marginsAdded(static_cast<PointingTool*>(tool)->size() * QMarginsF(1,1,1,1)), QGraphicsScene::ForegroundLayer);
        }
    }
}

void SlideScene::tabletRelease(const QPointF &pos, const QTabletEvent *event)
{
    if (current_tool)
        stopInputEvent(pos);
    else
    {
        Tool *tool = preferences()->currentTool(
                    tablet_device_to_input_device.value(event->pointerType())
                );
        if (tool && (tool->tool() & AnyPointingTool) && !(tool->device() & TabletNoPressure))
        {
            static_cast<PointingTool*>(tool)->setPos({0,0});
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
        }
    }
}

void SlideScene::startInputEvent(Tool *tool, const QPointF &pos, const float pressure)
{
    if (!tool || !(tool->tool() & AnyDrawTool))
        return;
    debug_verbose(DebugDrawing) << "Start input event" << tool->tool() << tool->device() << tool << pressure;
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
        if (tool->tool() == Pen && (tool->device() & preferences()->pressure_sensitive_input_devices))
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
    debug_verbose(DebugDrawing) << "Step input event" << current_tool->tool() << current_tool->device() << current_tool << pressure;
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
            invalidate(item->boundingRect(), QGraphicsScene::ItemLayer);
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

bool SlideScene::stopInputEvent(const QPointF &pos)
{
    if (current_tool)
    {
        debug_verbose(DebugDrawing) << "Stop input event" << current_tool->tool() << current_tool->device() << current_tool;
        const bool changes = currentPath && currentPath->size() > 1;
        stopDrawing();
        switch (current_tool->tool())
        {
        case Pen:
        case Highlighter:
            if (changes)
            {
                invalidate({QRect()}, QGraphicsScene::ItemLayer);
                current_tool = NULL;
                return true;
            }
            break;
        case Eraser:
        {
            current_tool = NULL;
            auto container = master->pathContainer(page | page_part);
            if (container)
                return container->applyMicroStep();
            break;
        }
        case NoTool:
            master->resolveLink(page, pos);
            break;
        default:
            break;
        }
        current_tool = NULL;
        return false;
    }
    master->resolveLink(page, pos);
    return false;
}
