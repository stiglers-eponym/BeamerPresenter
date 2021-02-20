#include "src/slidescene.h"
#include "src/pdfmaster.h"

SlideScene::SlideScene(const PdfMaster *master, const PagePart part, QObject *parent) :
    QGraphicsScene(parent),
    master(master),
    page_part(part)
{
    connect(this, &SlideScene::sendNewPath, master, &PdfMaster::receiveNewPath);
    connect(this, &SlideScene::requestPathContainer, master, &PdfMaster::requestPathContainer, Qt::DirectConnection);
}

SlideScene::~SlideScene()
{
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    for (auto &item : videoItems)
    {
        delete item.player;
        delete item.item;
    }
    videoItems.clear();
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
            break;
        if (tool->tool() & AnyDrawTool)
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
        break;
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
                if (static_cast<PointingTool*>(tool)->pos().size() == 1)
                {
                    const QPointF oldpos = static_cast<PointingTool*>(tool)->pos().first();
                    const QPointF &newpos = mouseevent->scenePos();
                    static_cast<PointingTool*>(tool)->setPos(newpos);
                    invalidate(QRectF(oldpos, newpos).normalized().marginsAdded(static_cast<PointingTool*>(tool)->size() * QMarginsF(1,1,1,1)), QGraphicsScene::ForegroundLayer);
                }
                else
                {
                    static_cast<PointingTool*>(tool)->setPos(mouseevent->scenePos());
                    invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
                }
                event->accept();
                return true;
            }
        }
        break;
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
            static_cast<PointingTool*>(tool)->clearPos();
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
        }
        else if (tool && tool->tool() == TextInputTool)
        {
            QGraphicsScene::event(event);
            if (!isTextEditing())
            {
                QGraphicsTextItem *item = new QGraphicsTextItem();
                item->setTextInteractionFlags(Qt::TextEditorInteraction);
                item->setFont(QFont(static_cast<const TextTool*>(tool)->font()));
                item->setDefaultTextColor(static_cast<const TextTool*>(tool)->color());
                addItem(item);
                item->show();
                item->setPos(mouseevent->scenePos());
                emit sendNewPath(page | page_part, item);
                item->setFocus();
            }
        }
        else if (!tool && mouseevent->button() == Qt::LeftButton)
            noToolClicked(mouseevent->scenePos(), mouseevent->buttonDownScenePos(Qt::LeftButton));
        event->accept();
        return true;
    }
    case QEvent::TouchBegin:
    {
        Tool *const tool = preferences()->currentTool(TouchInput);
        if (!tool)
            break;
        const auto touchevent = static_cast<QTouchEvent*>(event);
        if ((tool->tool() & AnyDrawTool) && (touchevent->touchPoints().size() == 1))
        {
            const QTouchEvent::TouchPoint &point = touchevent->touchPoints().first();
            startInputEvent(tool, point.scenePos(), point.pressure());
            event->accept();
            return true;
        }
        else if (tool->tool() & AnyPointingTool)
        {
            static_cast<PointingTool*>(tool)->clearPos();
            for (const auto &point : touchevent->touchPoints())
                static_cast<PointingTool*>(tool)->addPos(point.scenePos());
            invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
            event->accept();
            return true;
        }
        break;
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
                static_cast<PointingTool*>(tool)->clearPos();
                for (const auto &point : touchevent->touchPoints())
                    static_cast<PointingTool*>(tool)->addPos(point.scenePos());
                invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
                event->accept();
                return true;
            }
        }
        break;
    }
    case QEvent::TouchCancel:
        if (current_tool && (current_tool->device() & TouchInput))
        {
            if (stopInputEvent(QPointF()))
            {
                PathContainer *container;
                emit requestPathContainer(&container, page | page_part);
                if (container)
                    container->undo(this);
            }
            Tool *tool = preferences()->currentTool(TouchInput);
            if (tool && (tool->tool() & AnyPointingTool))
            {
                static_cast<PointingTool*>(tool)->clearPos();
                invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            }
            event->accept();
            return true;
        }
        break;
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
                static_cast<PointingTool*>(tool)->clearPos();
                invalidate(QRect(), QGraphicsScene::ForegroundLayer);
            }
            else if (tool && tool->tool() == TextInputTool && touchevent->touchPoints().size() == 1)
            {
                QGraphicsScene::event(event);
                if (!isTextEditing())
                {
                    QGraphicsTextItem *item = new QGraphicsTextItem();
                    item->setTextInteractionFlags(Qt::TextEditorInteraction);
                    item->setFont(QFont(static_cast<const TextTool*>(tool)->font()));
                    item->setDefaultTextColor(static_cast<const TextTool*>(tool)->color());
                    addItem(item);
                    item->show();
                    item->setPos(touchevent->touchPoints().first().scenePos());
                    emit sendNewPath(page | page_part, item);
                    item->setFocus();
                }
            }
            else if (!tool && touchevent->touchPoints().length() == 1)
                noToolClicked(touchevent->touchPoints().first().scenePos(), touchevent->touchPoints().first().startScenePos());
            event->accept();
            return true;
        }
    default:
        break;
    }
    return QGraphicsScene::event(event);
}

void SlideScene::receiveAction(const Action action)
{
    switch (action)
    {
    case ScrollDown:
        setSceneRect(sceneRect().translated(0, sceneRect().height()/5));
        break;
    case ScrollUp:
        setSceneRect(sceneRect().translated(0, -sceneRect().height()/5));
        break;
    case ScrollNormal:
        setSceneRect({{0,0}, sceneRect().size()});
        break;
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
    if ((flags & ShowTransitions) && (!newscene || newscene == this))
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
        if (flags & LoadAnyMedia)
            loadMedia(page);
        if (flags & ShowDrawings)
        {
            PathContainer *paths;
            emit requestPathContainer(&paths, page | page_part);
            if (paths)
            {
                const auto end = paths->cend();
                for (auto it = paths->cbegin(); it != end; ++it)
                    addItem(*it);
            }
        }
    }
    invalidate();
}

void SlideScene::loadMedia(const int page)
{
    QList<MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : qAsConst(*list))
    {
        switch (annotation.type)
        {
        case MediaAnnotation::VideoAnnotation:
        {
            debug_msg(DebugMedia) << "loading video" << annotation.file << annotation.rect;
            VideoItem &item = getVideoItem(annotation);
            item.item->setSize(item.annotation.rect.size());
            item.item->setPos(item.annotation.rect.topLeft());
            item.item->show();
            addItem(item.item);
            if (flags & AutoplayVideos)
            {
                debug_msg(DebugMedia) << "play video" << item.player->position() << item.player->state();
                item.player->play();
            }
            break;
        }
        case MediaAnnotation::AudioAnnotation:
        {
            qWarning() << "Sound annotation: not implemented yet";
            break;
        }
        case MediaAnnotation::InvalidAnnotation:
            break;
        }
    }
}

void SlideScene::cacheMediaNextPage()
{
    int newpage = page + 1;
    if (shift & AnyOverlay)
        newpage = master->getDocument()->overlaysShifted(page, 1 | (shift & AnyOverlay));
    cacheMedia(newpage);
}

void SlideScene::cacheMedia(int page)
{
    QList<MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : qAsConst(*list))
    {
        switch (annotation.type)
        {
        case MediaAnnotation::VideoAnnotation:
            debug_msg(DebugMedia) << "try to cache video" << annotation.file << annotation.rect;
            getVideoItem(annotation);
            break;
        case MediaAnnotation::AudioAnnotation:
            qWarning() << "Sound annotation: not implemented yet";
            break;
        case MediaAnnotation::InvalidAnnotation:
            break;
        }
    }
}

SlideScene::VideoItem &SlideScene::getVideoItem(const MediaAnnotation &annotation)
{
    for (auto &videoitem : videoItems)
    {
        if (videoitem.annotation == annotation && videoitem.item)
        {
            debug_msg(DebugMedia) << "Found video in cache" << annotation.file << annotation.rect;
            return videoitem;
        }
    }
    debug_msg(DebugMedia) << "Loading new video" << annotation.file << annotation.rect;
    QMediaPlayer *player = new QMediaPlayer(this);
    QGraphicsVideoItem *item = new QGraphicsVideoItem;
    connect(item, &QGraphicsVideoItem::destroyed, player, &QMediaPlayer::deleteLater);
    // Ugly fix to cache videos: show invisible video pixel
    item->setSize({1,1});
    item->setPos(sceneRect().bottomRight());
    addItem(item);
    item->show();
    player->setVideoOutput(item);
    player->setMedia(annotation.file);
    videoItems.append({annotation, item, player});
    return videoItems.last();
}

void SlideScene::startTransition(const int newpage, const SlideTransition &transition)
{
    // TODO!
    page = newpage;
    emit navigationToViews(page, this);
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    PathContainer *paths;
    emit requestPathContainer(&paths, page | page_part);
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
    else if (tool && tool->tool() == TextInputTool && !isTextEditing())
    {
        QGraphicsTextItem *item = new QGraphicsTextItem();
        item->setTextInteractionFlags(Qt::TextEditorInteraction);
        item->setFont(QFont(static_cast<const TextTool*>(tool)->font()));
        item->setDefaultTextColor(static_cast<const TextTool*>(tool)->color());
        addItem(item);
        item->show();
        item->setPos(pos);
        emit sendNewPath(page | page_part, item);
        item->setFocus();
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
            if (static_cast<PointingTool*>(tool)->pos().size() == 1)
            {
                const QPointF oldpos = static_cast<PointingTool*>(tool)->pos().last();
                static_cast<PointingTool*>(tool)->setPos(pos);
                invalidate(QRectF(oldpos, pos).normalized().marginsAdded(static_cast<PointingTool*>(tool)->size() * QMarginsF(1,1,1,1)), QGraphicsScene::ForegroundLayer);
            }
            else
            {
                static_cast<PointingTool*>(tool)->setPos(pos);
                invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
            }
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
            static_cast<PointingTool*>(tool)->clearPos();
            invalidate(QRect(), QGraphicsScene::ForegroundLayer);
        }
        else if (!tool)
            noToolClicked(pos);
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
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
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
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
        if (container)
            container->eraserMicroStep(pos, static_cast<DrawTool*>(current_tool)->width());
        break;
    }
    default:
        break;
    }
}

bool SlideScene::stopInputEvent(const QPointF &pos)
{
    if (!current_tool)
        return false;
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
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
        if (container)
            return container->applyMicroStep();
        break;
    }
    case NoTool:
        noToolClicked(pos);
        break;
    default:
        break;
    }
    current_tool = NULL;
    return false;
}

void SlideScene::noToolClicked(const QPointF &pos, const QPointF &startpos)
{
    // Try to handle multimedia annotation.
    for (auto &item : videoItems)
    {
        if (item.item && item.item->isVisible() && item.item->boundingRect().contains(pos))
        {
            if (startpos.isNull() || item.annotation.rect.contains(startpos))
            {
                if (item.player->state() == QMediaPlayer::PlayingState)
                    item.player->pause();
                else
                    item.player->play();
                return;
            }
            break;
        }
    }
    master->resolveLink(page, pos, startpos);
}
