#include <QDataStream>
#include <QRegularExpression>
#include <QXmlStreamWriter>
#include <QGuiApplication>
#include <QMimeData>
#include <QClipboard>
#include <QDesktopServices>
#if (QT_VERSION_MAJOR < 6)
#include <QMediaPlaylist>
#endif

#include "src/names.h"
#include "src/preferences.h"
#include "src/slidescene.h"
#include "src/slideview.h"
#include "src/pdfmaster.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/flexgraphicslineitem.h"
#include "src/drawing/pixmapgraphicsitem.h"
#include "src/drawing/rectgraphicsitem.h"
#include "src/drawing/ellipsegraphicsitem.h"
#include "src/drawing/linegraphicsitem.h"
#include "src/drawing/arrowgraphicsitem.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/selectiontool.h"
#include "src/drawing/pathcontainer.h"
#include "src/drawing/shaperecognizer.h"

SlideScene::SlideScene(const PdfMaster *master, const PagePart part, QObject *parent) :
    QGraphicsScene(parent),
    pageItem(new PixmapGraphicsItem(sceneRect())),
    master(master),
    page_part(part)
{
    connect(this, &SlideScene::sendNewPath, master, &PdfMaster::receiveNewPath, Qt::DirectConnection);
    connect(this, &SlideScene::replacePath, master, &PdfMaster::replacePath, Qt::DirectConnection);
    connect(this, &SlideScene::sendHistoryStep, master, &PdfMaster::addHistoryStep, Qt::DirectConnection);
    connect(this, &SlideScene::requestNewPathContainer, master, &PdfMaster::requestNewPathContainer, Qt::DirectConnection);
    connect(this, &SlideScene::sendRemovePaths, master, &PdfMaster::removeItems, Qt::DirectConnection);
    connect(this, &SlideScene::sendAddPaths, master, &PdfMaster::addItems, Qt::DirectConnection);
    connect(this, &SlideScene::selectionChanged, this, &SlideScene::updateSelectionRect, Qt::DirectConnection);
    pageItem->setZValue(-1e2);
    addItem(&selection_bounding_rect);
    addItem(pageItem);
    pageItem->show();
}

SlideScene::~SlideScene()
{
    delete animation;
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    delete pageItem;
    delete pageTransitionItem;
    for (const auto &media : mediaItems)
    {
        delete media.item;
        delete media.player;
#if (QT_VERSION_MAJOR >= 6)
        delete media.audio_out;
#endif
    }
    mediaItems.clear();
    delete currentlyDrawnItem;
    delete currentItemCollection;
}

void SlideScene::stopDrawing()
{
    debug_msg(DebugDrawing, "Stop drawing" << page << page_part);
    if (currentlyDrawnItem)
    {
        BasicGraphicsPath *newpath = NULL;
        switch (currentlyDrawnItem->type())
        {
        case BasicGraphicsPath::Type:
        case FullGraphicsPath::Type:
        {
            AbstractGraphicsPath *path = static_cast<AbstractGraphicsPath*>(currentlyDrawnItem);
            if (preferences()->global_flags & Preferences::FinalizeDrawnPaths)
                path->finalize();
            emit sendNewPath(page | page_part, currentlyDrawnItem);
            if (path->getTool().shape() == DrawTool::Recognize)
            {
                ShapeRecognizer recognizer(path);
                newpath = recognizer.recognize();
                if (newpath)
                {
                    addItem(newpath);
                    emit replacePath(page | page_part, currentlyDrawnItem, newpath);
                    currentlyDrawnItem = newpath;
                }
            }
            currentlyDrawnItem->show();
            invalidate(currentlyDrawnItem->sceneBoundingRect(), QGraphicsScene::ItemLayer);
            currentlyDrawnItem = NULL;
            break;
        }
        case RectGraphicsItem::Type:
            newpath = static_cast<RectGraphicsItem*>(currentlyDrawnItem)->toPath();
            break;
        case EllipseGraphicsItem::Type:
            newpath = static_cast<EllipseGraphicsItem*>(currentlyDrawnItem)->toPath();
            break;
        case LineGraphicsItem::Type:
            newpath = static_cast<LineGraphicsItem*>(currentlyDrawnItem)->toPath();
            break;
        case ArrowGraphicsItem::Type:
            newpath = static_cast<ArrowGraphicsItem*>(currentlyDrawnItem)->toPath();
            break;
        default:
            break;
        }
        if (currentlyDrawnItem != NULL)
        {
            removeItem(currentlyDrawnItem);
            delete currentlyDrawnItem;
            if (newpath)
            {
                addItem(newpath);
                newpath->show();
                emit sendNewPath(page | page_part, newpath);
            }
        }
        currentlyDrawnItem = NULL;
    }
    if (currentItemCollection)
    {
        removeItem(currentItemCollection);
        delete currentItemCollection;
        currentItemCollection = NULL;
    }
}

bool SlideScene::event(QEvent* event)
{
    debug_verbose(DebugDrawing, event);
    int device = 0;
    QList<QPointF> pos;
    QPointF start_pos;
    switch (event->type())
    {
    case QEvent::GraphicsSceneMousePress:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        device = (mouseevent->buttons() << 1) | Tool::StartEvent;
        pos.append(mouseevent->scenePos());
        break;
    }
    case QEvent::GraphicsSceneMouseMove:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        device = (mouseevent->buttons() ? mouseevent->buttons() << 1 : 1) | Tool::UpdateEvent;
        pos.append(mouseevent->scenePos());
        break;
    }
    case QEvent::GraphicsSceneMouseRelease:
    {
        const auto *mouseevent = static_cast<QGraphicsSceneMouseEvent*>(event);
        device = (mouseevent->button() << 1) | Tool::StopEvent;
        pos.append(mouseevent->scenePos());
        start_pos = mouseevent->buttonDownScenePos(mouseevent->button());
        break;
    }
    case QEvent::TouchBegin:
    {
        device = int(Tool::TouchInput) | Tool::StartEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
#if (QT_VERSION_MAJOR >= 6)
        for (const auto &point : touchevent->points())
            pos.append(point.scenePosition());
#else
        for (const auto &point : touchevent->touchPoints())
            pos.append(point.scenePos());
#endif
        break;
    }
    case QEvent::TouchUpdate:
    {
        device = int(Tool::TouchInput) | Tool::UpdateEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
#if (QT_VERSION_MAJOR >= 6)
        for (const auto &point : touchevent->points())
            pos.append(point.scenePosition());
#else
        for (const auto &point : touchevent->touchPoints())
            pos.append(point.scenePos());
#endif
        break;
    }
    case QEvent::TouchEnd:
    {
        device = int(Tool::TouchInput) | Tool::StopEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
#if (QT_VERSION_MAJOR >= 6)
        if (touchevent->points().size() > 0)
        {
            for (const auto &point : touchevent->points())
                pos.append(point.scenePosition());
            start_pos = touchevent->points().constFirst().scenePressPosition();
        }
#else
        if (touchevent->touchPoints().size() > 0)
        {
            for (const auto &point : touchevent->touchPoints())
                pos.append(point.scenePos());
            start_pos = touchevent->touchPoints().constFirst().startScenePos();
        }
#endif
        break;
    }
    case QEvent::TouchCancel:
        device = int(Tool::TouchInput) | Tool::CancelEvent;
        break;
    case QEvent::Leave:
        for (auto tool : qAsConst(preferences()->current_tools))
        {
            if (tool && tool->tool() == Tool::Pointer)
            {
                PointingTool *ptool = static_cast<PointingTool*>(tool);
                if (ptool->pos().isEmpty())
                    continue;
                QRectF rect({0,0}, ptool->size()*QSizeF(2,2));
                rect.moveCenter(ptool->pos().constFirst());
                ptool->clearPos();
                invalidate(rect);
                break;
            }
        }
        [[clang::fallthrough]];
    default:
        return QGraphicsScene::event(event);
    }
    event->accept();
    handleEvents(device, pos, start_pos, 1.);
    return true;
}

void SlideScene::handleEvents(const int device, const QList<QPointF> &pos, const QPointF &start_pos, const float pressure)
{
    Tool *tool = preferences()->currentTool(device & Tool::AnyDevice);
    if (!tool)
    {
        if ((device & Tool::AnyEvent) == Tool::StopEvent && pos.size() == 1)
            noToolClicked(pos.constFirst(), start_pos);
        return;
    }

    debug_verbose(DebugDrawing, "Handling event" << tool->tool() << tool->device() << device);
    if (tool->tool() & Tool::AnyDrawTool)
    {
        // TODO: multi-touch for draw tools
        switch (device & Tool::AnyEvent)
        {
        case Tool::UpdateEvent:
            stepInputEvent(static_cast<const DrawTool*>(tool), pos.constFirst(), pressure);
            break;
        case Tool::StartEvent:
            startInputEvent(static_cast<const DrawTool*>(tool), pos.constFirst(), pressure);
            break;
        case Tool::StopEvent:
            stopInputEvent(static_cast<const DrawTool*>(tool));
            break;
        case Tool::CancelEvent:
            if (stopInputEvent(static_cast<const DrawTool*>(tool)))
            {
                PathContainer *container = master->pathContainer(page | page_part);
                if (container)
                    container->undo(this);
                break;
            }
        }
    }
    else if (tool->tool() & Tool::AnyPointingTool)
    {
        PointingTool *ptool = static_cast<PointingTool*>(tool);
        ptool->scene() = this;
        switch (ptool->tool())
        {
        case Tool::Torch:
            if ((device & Tool::AnyEvent) == Tool::StopEvent)
                ptool->clearPos();
            else
                ptool->setPos(pos);
            invalidate();
            break;
        case Tool::Eraser:
        {
            PathContainer *container = master->pathContainer(page | page_part);
            if (container)
            {
                switch (device & Tool::AnyEvent)
                {
                case Tool::UpdateEvent:
                    for (const QPointF &point : pos)
                        container->eraserMicroStep(point, ptool->size());
                    break;
                case Tool::StartEvent:
                    container->startMicroStep();
                    for (const QPointF &point : pos)
                        container->eraserMicroStep(point, ptool->size());
                    break;
                case Tool::StopEvent:
                    if (container->applyMicroStep())
                        emit newUnsavedDrawings();
                    break;
                case Tool::CancelEvent:
                    if (container->applyMicroStep())
                    {
                        container->undo(this);
                        emit newUnsavedDrawings();
                    }
                    break;
                }
            }
            if (ptool->scale() <= 0.)
                break;
            [[clang::fallthrough]];
        }
        default:
        {
            QRectF point_rect = QRectF({0,0}, ptool->size()*QSize(2,2));
            for (auto point : ptool->pos())
            {
                point_rect.moveCenter(point);
                invalidate(point_rect, QGraphicsScene::ForegroundLayer);
            }
            if ((device & Tool::AnyEvent) == Tool::StopEvent && !(tool->device() & (Tool::TabletHover | Tool::MouseNoButton)))
                ptool->clearPos();
            else
            {
                ptool->setPos(pos);
                for (auto point : qAsConst(pos))
                {
                    point_rect.moveCenter(point);
                    invalidate(point_rect, QGraphicsScene::ForegroundLayer);
                }
            }
            break;
        }
        }
    }
    else if (tool->tool() & Tool::AnySelectionTool)
    {
        const QPointF &single_pos = pos.constFirst();
        SelectionTool *selection_tool = static_cast<SelectionTool*>(tool);
        switch (device & Tool::DeviceEventType::AnyEvent)
        {
        case Tool::DeviceEventType::StartEvent:
            handleSelectionStartEvents(selection_tool, single_pos);
            break;
        case Tool::DeviceEventType::UpdateEvent:
            handleSelectionUpdateEvents(selection_tool, single_pos, start_pos);
            break;
        case Tool::DeviceEventType::StopEvent:
            handleSelectionStopEvents(selection_tool, single_pos, start_pos);
            break;
        }
    }
    else if (tool->tool() == Tool::TextInputTool && (device & Tool::AnyEvent) == Tool::StopEvent && pos.size() == 1)
    {
        debug_msg(DebugDrawing, "Trying to start writing text" << (device & Tool::AnyDevice) << focusItem());
        for (auto item : static_cast<const QList<QGraphicsItem*>>(items(pos.constFirst())))
        {
            if (item->type() == TextGraphicsItem::Type)
            {
                setFocusItem(item);
                return;
            }
        }
        TextGraphicsItem *item = new TextGraphicsItem();
        item->setFont(QFont(static_cast<const TextTool*>(tool)->font()));
        item->setDefaultTextColor(static_cast<const TextTool*>(tool)->color());
        addItem(item);
        item->show();
        item->setPos(pos.constFirst());
        emit sendNewPath(page | page_part, NULL);
        PathContainer *container = master->pathContainer(page | page_part);
        if (container)
        {
            connect(item, &TextGraphicsItem::removeMe, container, &PathContainer::removeItem);
            connect(item, &TextGraphicsItem::addMe, container, &PathContainer::addTextItem);
        }
        setFocusItem(item);
    }
    else if ((device & Tool::AnyEvent) == Tool::StopEvent && pos.size() == 1)
        noToolClicked(pos.constFirst(), start_pos);
}

void SlideScene::handleSelectionStartEvents(SelectionTool *tool, const QPointF &pos)
{
    QList<QGraphicsItem*> selection = selectedItems();
    // Check if anything is selected.
    tool->reset();
    if (!selection.isEmpty())
    {
        selection.append(&selection_bounding_rect);
        tool->initTransformations(selection);
        // 1. Check if the user clicked on some special point on the
        // bounding rect of the selection.
        const QPolygonF selection_rect = selection_bounding_rect.scaleHandles();
        for (const auto &point : selection_rect)
            if ((pos - point).manhattanLength() < 4)
            {
                tool->startScaling(point, selection_bounding_rect.sceneCenter());
                return;
            }
        if ((pos - selection_bounding_rect.sceneRotationHandle()).manhattanLength() < 4)
        {
            tool->startRotation(pos, selection_bounding_rect.sceneCenter());
            return;
        }
        // 2. Check if the user clicked on a selected object.
        selection.pop_back();
        for (auto item : selection)
            if (item->contains(item->mapFromScene(pos)))
            {
                tool->startMove(pos);
                return;
            }
        selection.clear();
    }
    if (tool->type() == SelectionTool::NoOperation)
    {
        clearSelection();
        switch (tool->tool())
        {
        case Tool::BasicSelectionTool:
        {
            QGraphicsItem *item = itemAt(pos, QTransform());
            if (!(item && item->flags() & QGraphicsItem::ItemIsSelectable))
                break;
            item->setSelected(true);
            selection.append(item);
            selection.append(&selection_bounding_rect);
            tool->initTransformations(selection);
            tool->startMove(pos);
            break;
        }
        case Tool::RectSelectionTool:
            tool->startRectSelection(pos);
            break;
        case Tool::FreehandSelectionTool:
            // TODO: implement freehand selection tool
            break;
        default:
            break;
        }
    }
}

void SlideScene::handleSelectionUpdateEvents(SelectionTool *tool, const QPointF &pos, const QPointF &start_pos)
{
    switch (tool->type())
    {
    case SelectionTool::Move:
        tool->setLiveMoving(pos);
        break;
    case SelectionTool::Rotate:
        tool->setLiveRotation(pos);
        break;
    case SelectionTool::Resize:
        tool->setLiveScaling(pos);
        break;
    case SelectionTool::Select:
        // TODO: implement visualization for area selection tools
        break;
    default:
        break;
    }
}

void SlideScene::handleSelectionStopEvents(SelectionTool *tool, const QPointF &pos, const QPointF &start_pos)
{
    switch (tool->type())
    {
    case SelectionTool::Move:
    case SelectionTool::Rotate:
    case SelectionTool::Resize:
    {
        const QHash<QGraphicsItem*, QTransform> &originalTransforms = tool->originalTransforms();
        if (originalTransforms.count() <= 1)
            return;
        PathContainer::DrawHistoryStep *step = new PathContainer::DrawHistoryStep;
        const bool finalize = preferences()->global_flags & Preferences::FinalizeDrawnPaths;
        QTransform transform;
        for (auto it=originalTransforms.cbegin(); it!=originalTransforms.cend(); ++it)
        {
            if (it.key() == &selection_bounding_rect)
                continue;
            transform = it.key()->transform();
            transform *= it->inverted();
            step->transformedItems[it.key()] = transform;
            if (finalize && (it.key()->type() == BasicGraphicsPath::Type || it.key()->type() == FullGraphicsPath::Type))
                static_cast<AbstractGraphicsPath*>(it.key())->finalize();
        }
        emit sendHistoryStep(page | page_part, step);
        updateSelectionRect();
        break;
    }
    case SelectionTool::Select:
        switch (tool->tool())
        {
        case Tool::RectSelectionTool:
        {
            QPainterPath path;
            path.addRect(QRectF(tool->startPos(), pos).normalized());
            setSelectionArea(path, Qt::ReplaceSelection, Qt::ContainsItemShape);
            break;
        }
        case Tool::FreehandSelectionTool:
            // TODO: implement this
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void SlideScene::receiveAction(const Action action)
{
    debug_msg(DebugKeyInput, "SlideScene received action" << action);
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
    case PauseMedia:
        pauseMedia();
        break;
    case PlayMedia:
        playMedia();
        break;
    case PlayPauseMedia:
        playPauseMedia();
        break;
    case Mute:
        for (const auto &m : qAsConst(mediaItems))
#if (QT_VERSION_MAJOR >= 6)
            if (m.audio_out)
                m.audio_out->setMuted(true);
#else
            if (m.player)
                m.player->setMuted(true);
#endif
        break;
    case Unmute:
        for (const auto &m : qAsConst(mediaItems))
#if (QT_VERSION_MAJOR >= 6)
            if (m.audio_out)
                m.audio_out->setMuted(false);
#else
            if (m.player)
                m.player->setMuted(false);
#endif
        break;
    case CopyClipboard:
        if (slide_flags & ShowDrawings && hasFocus())
            copyToClipboard();
        break;
    case CutClipboard:
        if (slide_flags & ShowDrawings && hasFocus())
        {
            copyToClipboard();
            removeSelection();
        }
        break;
    case RemoveSelectedItems:
        if (slide_flags & ShowDrawings && hasFocus())
            removeSelection();
        break;
    case PasteClipboard:
        if (slide_flags & ShowDrawings && hasFocus())
            pasteFromClipboard();
        break;
    case DuplicateSelectedItems:
        if (slide_flags & ShowDrawings && hasFocus())
        {
            copyToClipboard();
            pasteFromClipboard();
        }
        break;
    case SelectionToForeground:
        if (slide_flags & ShowDrawings && hasFocus())
            selectionToForeground();
        break;
    case SelectAll:
        if (slide_flags & ShowDrawings && hasFocus())
            for (const auto item : items())
                if (item->flags() & QGraphicsItem::ItemIsSelectable)
                    item->setSelected(true);
        break;
    case ClearSelection:
        if (slide_flags & ShowDrawings && hasFocus())
            clearSelection();
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
    debug_verbose(DebugPageChange, newpage << pagesize << master->getDocument()->flexiblePageSizes());
    // Don't do anything if page size ist not valid. This avoids cleared slide
    // scenes which could mess up the layout and invalidate cache.
    if ((pagesize.isNull() || !pagesize.isValid()) && !master->getDocument()->flexiblePageSizes())
    {
        pageItem->clearPixmaps();
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
    debug_msg(DebugPageChange, "scene" << this << "navigates to" << newpage << "as" << newscene);
    pauseMedia();
    clearSelection();
    if (pageTransitionItem)
    {
        removeItem(pageTransitionItem);
        delete pageTransitionItem;
        pageTransitionItem = NULL;
    }
    if (animation)
    {
        animation->stop();
        delete animation;
        animation = NULL;
    }
    pageItem->setOpacity(1.);
    pageItem->setRect(sceneRect());
    pageItem->trackNew();
    if ((!newscene || newscene == this) && page != newpage && (slide_flags & ShowTransitions))
    {
        PdfDocument::SlideTransition transition = master->transition(newpage > page ? newpage : page);
        if (transition.type > 0)
        {
            if (newpage < page)
                transition.invert();
            debug_msg(DebugTransitions, "Transition:" << transition.type << transition.duration << transition.properties << transition.angle << transition.scale);
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
        addItem(pageItem);
        loadMedia(page);
        if (slide_flags & ShowDrawings)
        {
            PathContainer *paths;
            emit requestNewPathContainer(&paths, page | page_part);
            if (paths)
            {
                const auto end = paths->cend();
                for (auto it = paths->cbegin(); it != end; ++it)
                    addItem(*it);
            }
        }
    }
    invalidate();
    emit finishTransition();
}

void SlideScene::loadMedia(const int page)
{
    if (!(slide_flags & LoadMedia))
        return;
    const QList<PdfDocument::MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : *list)
    {
        if (annotation.type != PdfDocument::MediaAnnotation::InvalidAnnotation)
        {
            debug_msg(DebugMedia, "loading media" << annotation.file << annotation.rect);
            MediaItem &item = getMediaItem(annotation, page);
            if (item.item)
            {
                item.item->setSize(item.annotation.rect.size());
                item.item->setPos(item.annotation.rect.topLeft());
                item.item->show();
                addItem(item.item);
            }
            // TODO: control autoplay audio
            if (item.player && slide_flags & AutoplayVideo)
                item.player->play();
        }
    }
    delete list;
}

void SlideScene::postRendering()
{
    pageItem->clearOld();
    int newpage = page + 1;
    if (shift & AnyOverlay)
        newpage = master->getDocument()->overlaysShifted(page, 1 | (shift & AnyOverlay));
    if (slide_flags & CacheVideos)
        cacheMedia(newpage);
    // Clean up media
    if (mediaItems.size() > 2)
    {
        debug_verbose(DebugMedia, "Start cleaning up media" << mediaItems.size());
        for (auto &media : mediaItems)
        {
            if (media.player == NULL)
                continue;
            if (!media.pages.empty())
            {
                const auto it = media.pages.lower_bound(page);
                if ((it != media.pages.end() && *it <= newpage) || (it != media.pages.begin() && *(std::prev(it)) >= page-1))
                    continue;
            }
            debug_msg(DebugMedia, "Deleting media item:" << media.annotation.file << media.pages.size());
            delete media.player;
            media.player = NULL;
            delete media.item;
            media.item = NULL;
#if (QT_VERSION_MAJOR >= 6)
            delete media.audio_out;
            media.audio_out = NULL;
#endif
        }
    }
}

void SlideScene::cacheMedia(const int page)
{
    const QList<PdfDocument::MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : *list)
        if (annotation.type != PdfDocument::MediaAnnotation::InvalidAnnotation)
            getMediaItem(annotation, page);
    delete list;
}

SlideScene::MediaItem &SlideScene::getMediaItem(const PdfDocument::MediaAnnotation &annotation, const int page)
{
    for (auto &mediaitem : mediaItems)
    {
        if (mediaitem.annotation == annotation && mediaitem.player)
        {
            debug_msg(DebugMedia, "Found media in cache" << annotation.file << annotation.rect);
            mediaitem.pages.insert(page);
            return mediaitem;
        }
    }
    debug_msg(DebugMedia, "Loading new media" << annotation.file << annotation.rect);
    MediaPlayer *player = new MediaPlayer(this);
#if (QT_VERSION_MAJOR >= 6)
    QAudioOutput *audio_out = NULL;
    if (annotation.type & PdfDocument::MediaAnnotation::HasAudio)
    {
        audio_out = new QAudioOutput(this);
        if ((slide_flags & MuteSlide) || (preferences()->global_flags & Preferences::MuteApplication) || annotation.volume <= 0.)
            audio_out->setMuted(true);
        else
            audio_out->setVolume(annotation.volume);
        debug_msg(DebugMedia, "muted:" << audio_out->isMuted() << "volume:" << audio_out->volume());
    }
    player->setAudioOutput(audio_out);
#else
    if (annotation.type & PdfDocument::MediaAnnotation::HasAudio)
    {
        if ((slide_flags & MuteSlide) || (preferences()->global_flags & Preferences::MuteApplication) || annotation.volume <= 0.)
            player->setMuted(true);
        else
            player->setVolume(100*annotation.volume);
        debug_msg(DebugMedia, "muted:" << player->isMuted() << "volume:" << player->volume());
    }
#endif
    QGraphicsVideoItem *item = NULL;
    if (annotation.type & PdfDocument::MediaAnnotation::HasVideo)
    {
        item = new QGraphicsVideoItem;
        player->setVideoOutput(item);
#if (QT_VERSION_MAJOR < 6)
        // Ugly fix to cache videos: show invisible video pixel
        item->setSize({1,1});
        item->setPos(sceneRect().bottomRight());
        addItem(item);
        item->show();
#endif
    }
    if ((annotation.type & PdfDocument::MediaAnnotation::Embedded) == 0)
#if (QT_VERSION_MAJOR >= 6)
        player->setSource(annotation.file);
#else
    {
        QMediaPlaylist *playlist = new QMediaPlaylist(player);
        playlist->addMedia(annotation.file);
        player->setPlaylist(playlist);
    }
#endif
    else
    {
        warn_msg("Embedded media are currently not supported.");
        //QBuffer *buffer = new QBuffer(player);
        //buffer->setData(static_cast<const PdfDocument::EmbeddedMedia&>(annotation).data);
        //buffer->open(QBuffer::ReadOnly);
        //player->setSourceDevice(buffer);
    }
    switch (annotation.mode)
    {
    case PdfDocument::MediaAnnotation::Once:
    case PdfDocument::MediaAnnotation::Open:
        break;
    case PdfDocument::MediaAnnotation::Palindrome:
        warn_msg("Palindrome video: not implemented (yet)");
        // TODO
        [[clang::fallthrough]];
    case PdfDocument::MediaAnnotation::Repeat:
    default:
#if (QT_VERSION_MAJOR >= 6)
        connect(player, &MediaPlayer::mediaStatusChanged, player, &MediaPlayer::repeatIfFinished);
#else
        if (player->playlist())
            player->playlist()->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
#endif
        break;
    }
#if (QT_VERSION_MAJOR >= 6)
    mediaItems.append({annotation, item, player, audio_out, {page}});
#else
    mediaItems.append({annotation, item, player, {page}});
#endif
    return mediaItems.last();
}

void SlideScene::startTransition(const int newpage, const PdfDocument::SlideTransition &transition)
{
    pageTransitionItem = new PixmapGraphicsItem(sceneRect());
    for (const auto view : static_cast<const QList<QGraphicsView*>>(views()))
        static_cast<SlideView*>(view)->prepareTransition(pageTransitionItem);
    page = newpage;
    PixmapGraphicsItem *oldPage = pageTransitionItem;
    if ((transition.type == PdfDocument::SlideTransition::Fly || transition.type == PdfDocument::SlideTransition::FlyRectangle) && !views().isEmpty())
    {
        if (!(transition.properties & PdfDocument::SlideTransition::Outwards))
        {
            pageTransitionItem = new PixmapGraphicsItem(sceneRect());
            connect(pageTransitionItem, &QObject::destroyed, oldPage, &PixmapGraphicsItem::deleteLater);
            oldPage->setZValue(1e1);
        }
    }
    else
        emit navigationToViews(page, this);
    debug_msg(DebugTransitions, "transition:" << transition.type << transition.duration << transition.angle << transition.properties);
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    pageItem->setOpacity(1.);
    addItem(pageItem);
    loadMedia(page);
    if (slide_flags & ShowDrawings)
    {
        PathContainer *paths;
        emit requestNewPathContainer(&paths, page | page_part);
        if (paths)
        {
            const auto end = paths->cend();
            for (auto it = paths->cbegin(); it != end; ++it)
                addItem(*it);
        }
    }
    delete animation;
    animation = NULL;
    switch (transition.type)
    {
    case PdfDocument::SlideTransition::Split:
    {
        const bool outwards = transition.properties & PdfDocument::SlideTransition::Outwards;
        pageTransitionItem->setMaskType(outwards ? PixmapGraphicsItem::NegativeClipping : PixmapGraphicsItem::PositiveClipping);
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "mask");
        propanim->setDuration(1000*transition.duration);
        QRectF rect = sceneRect();
        if (outwards)
            propanim->setEndValue(rect);
        else
            propanim->setStartValue(rect);
        if (transition.properties & PdfDocument::SlideTransition::Vertical)
        {
            rect.moveTop(rect.top() + rect.height()/2);
            rect.setHeight(0.);
        }
        else
        {
            rect.moveLeft(rect.left() + rect.width()/2);
            rect.setWidth(0.);
        }
        if (outwards)
        {
            propanim->setStartValue(rect);
            pageTransitionItem->setMask(rect);
        }
        else
            propanim->setEndValue(rect);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Blinds:
    {
        const bool vertical = transition.properties & PdfDocument::SlideTransition::Vertical;
        pageTransitionItem->setMaskType(vertical ? PixmapGraphicsItem::VerticalBlinds : PixmapGraphicsItem::HorizontalBlinds);
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "mask");
        propanim->setDuration(1000*transition.duration);
        QRectF rect = sceneRect();
        if (vertical)
            rect.setWidth(rect.width()/BLINDS_NUMBER_V);
        else
            rect.setHeight(rect.height()/BLINDS_NUMBER_H);
        propanim->setStartValue(rect);
        if (vertical)
            rect.setWidth(0);
        else
            rect.setHeight(0);
        propanim->setEndValue(rect);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Box:
    {
        const bool outwards = transition.properties & PdfDocument::SlideTransition::Outwards;
        pageTransitionItem->setMaskType(outwards ? PixmapGraphicsItem::NegativeClipping : PixmapGraphicsItem::PositiveClipping);
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "mask");
        propanim->setDuration(1000*transition.duration);
        QRectF rect = sceneRect();
        if (outwards)
            propanim->setEndValue(rect);
        else
            propanim->setStartValue(rect);
        rect.moveTopLeft(rect.center());
        rect.setSize({0,0});
        if (outwards)
        {
            propanim->setStartValue(rect);
            pageTransitionItem->setMask(rect);
        }
        else
            propanim->setEndValue(rect);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Wipe:
    {
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "mask");
        pageTransitionItem->setMaskType(PixmapGraphicsItem::PositiveClipping);
        propanim->setDuration(1000*transition.duration);
        QRectF rect = sceneRect();
        pageTransitionItem->setMask(rect);
        propanim->setStartValue(rect);
        switch (transition.angle)
        {
        case 90:
            rect.setBottom(rect.top()+1);
            break;
        case 180:
            rect.setRight(rect.left()+1);
            break;
        case 270:
            rect.setTop(rect.bottom()-1);
            break;
        default:
            rect.setLeft(rect.right()-1);
            break;
        }
        propanim->setEndValue(rect);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Dissolve:
    {
        pageTransitionItem->setOpacity(0.);
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "opacity");
        propanim->setDuration(1000*transition.duration);
        propanim->setStartValue(1.);
        propanim->setEndValue(0.);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Glitter:
    {
        pageTransitionItem->setMaskType(PixmapGraphicsItem::Glitter);
        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "progress");
        propanim->setDuration(1000*transition.duration);
        propanim->setStartValue(GLITTER_NUMBER);
        propanim->setEndValue(0);
        propanim->setEasingCurve(QEasingCurve::InOutSine);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Fly:
    case PdfDocument::SlideTransition::FlyRectangle:
    {
        const bool outwards = transition.properties & PdfDocument::SlideTransition::Outwards;
        for (const auto &view : static_cast<const QList<QGraphicsView*>>(views()))
        {
            SlideView *slideview = static_cast<SlideView*>(view);
            slideview->pageChangedBlocking(page, this);
            slideview->prepareFlyTransition(outwards, oldPage, pageTransitionItem);
        }
        if (!outwards)
            addItem(oldPage);
        pageItem->setZValue(-1e4);
        pageTransitionItem->setZValue(1e5);

        QPropertyAnimation *propanim = new QPropertyAnimation(pageTransitionItem, "x");
        animation = propanim;
        propanim->setDuration(1000*transition.duration);
        switch (transition.angle)
        {
        case 90:
            propanim->setPropertyName("y");
            propanim->setStartValue(outwards ? 0. : sceneRect().height());
            propanim->setEndValue(outwards ? -sceneRect().height() : 0.);
            break;
        case 180:
            propanim->setStartValue(outwards ? 0. : sceneRect().width());
            propanim->setEndValue(outwards ? -sceneRect().width() : 0.);
            break;
        case 270:
            propanim->setPropertyName("y");
            propanim->setStartValue(outwards ? 0. : -sceneRect().height());
            propanim->setEndValue(outwards ? sceneRect().height() : 0.);
            break;
        default:
            propanim->setStartValue(outwards ? 0. : -sceneRect().width());
            propanim->setEndValue(outwards ? sceneRect().width() : 0.);
            break;
        }
        propanim->setEasingCurve(outwards ? QEasingCurve::InSine : QEasingCurve::OutSine);
        break;
    }
    case PdfDocument::SlideTransition::Push:
    {
        /* TODO: For push transitions the new page is not ready when
         * the animation starts. Instead of the new page, first the old
         * page is shown where the new page is expected. However, this
         * is usually only noted when the window geometry does not match
         * the slide geometry. */
        QPropertyAnimation *propanim = new QPropertyAnimation(this, "sceneRect");
        propanim->setDuration(1000*transition.duration);
        pageTransitionItem->setZValue(-1e3);
        QRectF movedrect = sceneRect();
        switch (transition.angle)
        {
        case 90:
            movedrect.moveBottom(movedrect.top());
            break;
        case 180:
            movedrect.moveRight(movedrect.left());
            break;
        case 270:
            movedrect.moveTop(movedrect.bottom());
            break;
        default:
            movedrect.moveLeft(movedrect.right());
            break;
        }
        pageTransitionItem->setRect(movedrect);
        propanim->setStartValue(movedrect);
        propanim->setEndValue(sceneRect());
        propanim->setEasingCurve(QEasingCurve::InOutSine);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Cover:
    {
        QParallelAnimationGroup *groupanim = new QParallelAnimationGroup();
        QPropertyAnimation *sceneanim = new QPropertyAnimation(this, "sceneRect", groupanim);
        QPropertyAnimation *bganim = new QPropertyAnimation(pageTransitionItem, "x", groupanim);
        sceneanim->setDuration(1000*transition.duration);
        bganim->setDuration(1000*transition.duration);
        pageTransitionItem->setZValue(-1e3);
        QRectF movedrect = sceneRect();
        switch (transition.angle)
        {
        case 90:
            bganim->setPropertyName("y");
            movedrect.moveBottom(movedrect.top());
            bganim->setStartValue(movedrect.y());
            bganim->setEndValue(sceneRect().y());
            break;
        case 180:
            movedrect.moveRight(movedrect.left());
            bganim->setStartValue(movedrect.x());
            bganim->setEndValue(sceneRect().x());
            break;
        case 270:
            bganim->setPropertyName("y");
            movedrect.moveTop(movedrect.bottom());
            bganim->setStartValue(movedrect.y());
            bganim->setEndValue(sceneRect().y());
            break;
        default:
            movedrect.moveLeft(movedrect.right());
            bganim->setStartValue(movedrect.x());
            bganim->setEndValue(sceneRect().x());
            break;
        }
        sceneanim->setStartValue(movedrect);
        sceneanim->setEndValue(sceneRect());
        groupanim->addAnimation(sceneanim);
        groupanim->addAnimation(bganim);
        sceneanim->setEasingCurve(QEasingCurve::OutSine);
        bganim->setEasingCurve(QEasingCurve::OutSine);
        animation = groupanim;
        break;
    }
    case PdfDocument::SlideTransition::Uncover:
    {
        QPropertyAnimation *propanim = new QPropertyAnimation();
        propanim->setDuration(1000*transition.duration);
        switch (transition.angle)
        {
        case 90:
            propanim->setPropertyName("y");
            propanim->setStartValue(0.);
            propanim->setEndValue(-sceneRect().height());
            break;
        case 180:
            propanim->setPropertyName("x");
            propanim->setStartValue(0.);
            propanim->setEndValue(-sceneRect().width());
            break;
        case 270:
            propanim->setPropertyName("y");
            propanim->setStartValue(0.);
            propanim->setEndValue(sceneRect().height());
            break;
        default:
            propanim->setPropertyName("x");
            propanim->setStartValue(0.);
            propanim->setEndValue(sceneRect().width());
            break;
        }
        propanim->setTargetObject(pageTransitionItem);
        propanim->setEasingCurve(QEasingCurve::InSine);
        animation = propanim;
        break;
    }
    case PdfDocument::SlideTransition::Fade:
    {
        pageTransitionItem->setOpacity(0.);
        QParallelAnimationGroup *groupanim = new QParallelAnimationGroup();
        QPropertyAnimation *oldpageanim = new QPropertyAnimation(pageTransitionItem, "opacity", groupanim);
        QPropertyAnimation *newpageanim = new QPropertyAnimation(pageItem, "opacity", groupanim);
        oldpageanim->setDuration(1000*transition.duration);
        oldpageanim->setStartValue(1.);
        oldpageanim->setEndValue(0.);
        newpageanim->setDuration(1000*transition.duration);
        newpageanim->setStartValue(0.);
        newpageanim->setEndValue(1.);
        groupanim->addAnimation(oldpageanim);
        groupanim->addAnimation(newpageanim);
        oldpageanim->setEasingCurve(QEasingCurve::OutQuart);
        newpageanim->setEasingCurve(QEasingCurve::InQuart);
        animation = groupanim;
        break;
    }
    }
    if (animation)
    {
        connect(animation, &QAbstractAnimation::finished, this, &SlideScene::endTransition);
        addItem(pageTransitionItem);
        animation->start(QAbstractAnimation::KeepWhenStopped);
    }
}

void SlideScene::endTransition()
{
    pageItem->setOpacity(1.);
    if (pageTransitionItem)
    {
        removeItem(pageTransitionItem);
        delete pageTransitionItem;
        pageTransitionItem = NULL;
    }
    if (animation)
    {
        animation->stop();
        delete animation;
        animation = NULL;
    }
    loadMedia(page);
    invalidate();
    emit finishTransition();
}

void SlideScene::tabletPress(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                int(event->pressure() > 0 ? tablet_device_to_input_device.value(event->pointerType()) : Tool::TabletHover) | Tool::StartEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                int(event->pressure() > 0 ? tablet_device_to_input_device.value(event->pointerType()) : Tool::TabletHover) | Tool::UpdateEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::tabletRelease(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                int(tablet_device_to_input_device.value(event->pointerType())) | Tool::StopEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::startInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure)
{
    if (!tool || !(tool->tool() & Tool::AnyDrawTool) || !(slide_flags & ShowDrawings))
        return;
    debug_verbose(DebugDrawing, "Start input event" << tool->tool() << tool->device() << tool << pressure);
    stopDrawing();
    if (currentItemCollection || currentlyDrawnItem)
        return;
    clearSelection();
    currentItemCollection = new QGraphicsItemGroup();
    addItem(currentItemCollection);
    currentItemCollection->show();
    switch (tool->shape()) {
    case DrawTool::Freehand:
    case DrawTool::Recognize:
        if (tool->tool() == Tool::Pen && (tool->device() & Tool::PressureSensitiveDevices))
            currentlyDrawnItem = new FullGraphicsPath(*tool, pos, pressure);
        else
            currentlyDrawnItem = new BasicGraphicsPath(*tool, pos);
        currentlyDrawnItem->hide();
        break;
    case DrawTool::Rect:
    {
        RectGraphicsItem *rect_item = new RectGraphicsItem(*tool, pos);
        rect_item->show();
        currentlyDrawnItem = rect_item;
        break;
    }
    case DrawTool::Ellipse:
    {
        EllipseGraphicsItem *rect_item = new EllipseGraphicsItem(*tool, pos);
        rect_item->show();
        currentlyDrawnItem = rect_item;
        break;
    }
    case DrawTool::Line:
    {
        LineGraphicsItem *line_item = new LineGraphicsItem(*tool, pos);
        line_item->show();
        currentlyDrawnItem = line_item;
        break;
    }
    case DrawTool::Arrow:
    {
        ArrowGraphicsItem *arrow_item = new ArrowGraphicsItem(*tool, pos);
        arrow_item->show();
        currentlyDrawnItem = arrow_item;
        break;
    }
    }
    addItem(currentlyDrawnItem);
}

void SlideScene::stepInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure)
{
    if (pressure <= 0 || !tool || !(slide_flags & ShowDrawings))
        return;
    debug_verbose(DebugDrawing, "Step input event" << tool->tool() << tool->device() << tool << pressure);
    if (!currentlyDrawnItem)
        return;
    switch (currentlyDrawnItem->type())
    {
        case BasicGraphicsPath::Type:
        {
            if (!currentItemCollection)
                break;
            BasicGraphicsPath *current_path = static_cast<BasicGraphicsPath*>(currentlyDrawnItem);
            if (current_path->getTool() != *tool)
                break;
            FlexGraphicsLineItem *item = new FlexGraphicsLineItem(QLineF(current_path->lastPoint(), pos), tool->compositionMode());
            current_path->addPoint(current_path->mapFromScene(pos));
            item->setPen(tool->pen());
            item->show();
            addItem(item);
            currentItemCollection->addToGroup(item);
            currentItemCollection->show();
            invalidate(item->sceneBoundingRect(), QGraphicsScene::ItemLayer);
            break;
        }
        case FullGraphicsPath::Type:
        {
            if (!currentItemCollection)
                break;
            FullGraphicsPath *current_path = static_cast<FullGraphicsPath*>(currentlyDrawnItem);
            if (current_path->getTool() != *tool)
                break;
            FlexGraphicsLineItem *item = new FlexGraphicsLineItem(QLineF(current_path->lastPoint(), pos), tool->compositionMode());
            current_path->addPoint(current_path->mapFromScene(pos), pressure);
            QPen pen = tool->pen();
            pen.setWidthF(pen.widthF() * pressure);
            item->setPen(pen);
            item->show();
            addItem(item);
            currentItemCollection->addToGroup(item);
            currentItemCollection->show();
            invalidate(item->sceneBoundingRect(), QGraphicsScene::ItemLayer);
            break;
        }
        case RectGraphicsItem::Type:
            static_cast<RectGraphicsItem*>(currentlyDrawnItem)->setSecondPoint(pos);
            break;
        case EllipseGraphicsItem::Type:
            static_cast<EllipseGraphicsItem*>(currentlyDrawnItem)->setSecondPoint(pos);
            break;
        case LineGraphicsItem::Type:
            static_cast<LineGraphicsItem*>(currentlyDrawnItem)->setSecondPoint(pos);
            break;
        case ArrowGraphicsItem::Type:
            static_cast<ArrowGraphicsItem*>(currentlyDrawnItem)->setSecondPoint(pos);
            break;
    }
}

bool SlideScene::stopInputEvent(const DrawTool *tool)
{
    if (!tool || !(slide_flags & ShowDrawings))
        return false;
    debug_verbose(DebugDrawing, "Stop input event" << tool->tool() << tool->device() << tool);
    const bool changes = currentlyDrawnItem != NULL;
    stopDrawing();
    if (changes)
    {
        invalidate({QRect()}, QGraphicsScene::ItemLayer);
        return true;
    }
    return false;
}

void SlideScene::noToolClicked(const QPointF &pos, const QPointF &startpos)
{
    debug_verbose(DebugMedia, "Clicked without tool" << pos << startpos);
    // Try to handle multimedia annotation.
    for (auto &item : mediaItems)
    {
#if __cplusplus >= 202002L
        // std::set<T>.contains is only available since C++ 20
        if (item.pages.contains(page &~NotFullPage) && item.annotation.rect.contains(pos) && item.player)
#else
        if (item.pages.find(page &~NotFullPage) != item.pages.end() && item.annotation.rect.contains(pos) && item.player)
#endif
        {
            if (startpos.isNull() || item.annotation.rect.contains(startpos))
            {
#if (QT_VERSION_MAJOR >= 6)
                if (item.player->playbackState() == QMediaPlayer::PlayingState)
#else
                if (item.player->state() == QMediaPlayer::PlayingState)
#endif
                    item.player->pause();
                else
                    item.player->play();
                return;
            }
            break;
        }
    }
    const PdfDocument::PdfLink *link = master->getDocument()->linkAt(page, pos);
    if (link && (startpos.isNull() || link->area.contains(startpos)))
    {
        switch (link->type)
        {
        case PdfDocument::PdfLink::PageLink:
            emit navigationSignal(static_cast<const PdfDocument::GotoLink*>(link)->page);
            break;
        case PdfDocument::PdfLink::ActionLink:
            emit sendAction(static_cast<const PdfDocument::ActionLink*>(link)->action);
            break;
        case PdfDocument::PdfLink::RemoteUrl:
        case PdfDocument::PdfLink::LocalUrl:
        case PdfDocument::PdfLink::ExternalPDF:
            QDesktopServices::openUrl(static_cast<const PdfDocument::ExternalLink*>(link)->url);
            break;
        case PdfDocument::PdfLink::SoundLink:
        case PdfDocument::PdfLink::MovieLink:
        {
            // This is untested!
            MediaItem &item = getMediaItem(static_cast<const PdfDocument::MediaLink*>(link)->annotation, page);
            if (item.item)
            {
                item.item->setSize(item.annotation.rect.size());
                item.item->setPos(item.annotation.rect.topLeft());
                item.item->show();
                addItem(item.item);
            }
            if (item.player)
                item.player->play();
            break;
        }
        case PdfDocument::PdfLink::NoLink:
            break;
        }
    }
    delete link;
}

void SlideScene::createSliders() const
{
    for (auto &item : mediaItems)
    {
#if __cplusplus >= 202002L
        if (item.pages.contains(page &~NotFullPage) && item.player)
#else
        if (item.pages.find(page &~NotFullPage) != item.pages.end() && item.player)
#endif
        {
            for (const auto view : static_cast<const QList<QGraphicsView*>>(views()))
                static_cast<SlideView*>(view)->addMediaSlider(item);
        }
    }
}

void SlideScene::playMedia() const
{
    for (auto &item : mediaItems)
    {
#if __cplusplus >= 202002L
        if (item.pages.contains(page &~NotFullPage) && item.player)
#else
        if (item.pages.find(page &~NotFullPage) != item.pages.end() && item.player)
#endif
            item.player->play();
    }
}

void SlideScene::pauseMedia() const
{
    for (auto &item : mediaItems)
    {
#if __cplusplus >= 202002L
        if (item.pages.contains(page &~NotFullPage) && item.player)
#else
        if (item.pages.find(page &~NotFullPage) != item.pages.end() && item.player)
#endif
            item.player->pause();
    }
}

void SlideScene::playPauseMedia() const
{
    for (auto &item : mediaItems)
    {
        if (
#if __cplusplus >= 202002L
                item.pages.contains(page &~NotFullPage)
#else
                item.pages.find(page &~NotFullPage) != item.pages.end()
#endif
                && item.player
#if (QT_VERSION_MAJOR >= 6)
                && item.player->playbackState() == QMediaPlayer::PlayingState
#else
                && item.player->state() == QMediaPlayer::PlayingState
#endif
            )
        {
            pauseMedia();
            return;
        }
    }
    playMedia();
}

void SlideScene::updateSelectionRect() noexcept
{
    // TODO: only call manually for higher efficiency?
    // (note: selection can also be changed by eraser / undo / redo / ...)
    QList<QGraphicsItem*> items = selectedItems();
    if (items.isEmpty())
    {
        selection_bounding_rect.hide();
        return;
    }
    // TODO: This is probably quite slow
    QRectF newrect = items.first()->mapToScene(items.first()->shape()).controlPointRect();
    for (const auto &item : items)
        newrect = newrect.united(item->mapToScene(item->shape()).controlPointRect());
    selection_bounding_rect.setRect(newrect);
    if (selection_bounding_rect.scene() != this)
        addItem(&selection_bounding_rect);
    selection_bounding_rect.show();
}

void SlideScene::removeSelection() const
{
    const QList<QGraphicsItem*> selection = selectedItems();
    emit sendRemovePaths(page | page_part, selection);
}

void SlideScene::copyToClipboard() const
{
    const QList<QGraphicsItem*> selection = selectedItems();
    if (selection.isEmpty())
        return;
    // Write to native data type
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << selection;
    QMimeData *mimedata = new QMimeData();
    mimedata->setData("application/beamerpresenter", data);
    data.clear();
    // Write svg data
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(0);
    writer.writeStartDocument();
    writer.writeComment(" Created with BeamerPresenter ");
    writer.writeStartElement("svg");
    writer.writeAttribute("version", "1.1");
    const QRectF rect = selection_bounding_rect.sceneBoundingRect();
    writer.writeAttribute("width", QString::number(rect.width()));
    writer.writeAttribute("height", QString::number(rect.height()));
    for (auto &item : selection)
    {
        const QTransform transform = item->transform();
        QString matrix = QString("matrix(%1,%2,%3,%4,%5,%6)")
                .arg(transform.m11())
                .arg(transform.m12())
                .arg(transform.m21())
                .arg(transform.m22())
                .arg(transform.dx())
                .arg(transform.dy());
        switch (item->type())
        {
        case BasicGraphicsPath::Type:
        case FullGraphicsPath::Type:
        {
            // TODO: flexible width of FullGraphicsPath is ignored.
            const AbstractGraphicsPath *path = static_cast<const AbstractGraphicsPath*>(item);
            writer.writeStartElement("g");
            writer.writeAttribute("x", QString::number(item->x()));
            writer.writeAttribute("y", QString::number(item->y()));
            writer.writeAttribute("transform", matrix);
            writer.writeStartElement("path");
            const DrawTool &tool = path->getTool();
            QString style = "fill:";
            style += tool.brush().style() == Qt::NoBrush ? "none" : tool.brush().color().name();
            style += ";stroke:";
            style += tool.color().name();
            style += ";stroke-width:";
            style += QString::number(tool.width());
            style += ";stroke-linecap:round;stroke-linejoin:round;";
            writer.writeAttribute("style", style);
            writer.writeAttribute("d", path->svgCoordinates());
            writer.writeEndElement();// "path" element
            writer.writeEndElement(); // "g" element
            break;
        }
        case TextGraphicsItem::Type:
        {
            const TextGraphicsItem *textItem = static_cast<const TextGraphicsItem*>(item);
            writer.writeStartElement("text");
            const QPointF origin = item->sceneBoundingRect().bottomLeft();
            // TODO: coordinates are shifted.
            writer.writeAttribute("x", QString::number(origin.x()));
            writer.writeAttribute("y", QString::number(origin.y()));
            writer.writeAttribute("transform", matrix);
            writer.writeAttribute("style", "font-size:" + QString::number(textItem->font().pointSizeF()) + "pt");
            // TODO: currently only plain text is supported.
            writer.writeCharacters(textItem->toPlainText());
            writer.writeEndElement();
            break;
        }
        }
    }
    writer.writeEndElement(); // "svg" element
    writer.writeEndDocument();
    mimedata->setData("image/svg+xml", data);
    // Add data to clipboard
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(mimedata);
}

void SlideScene::pasteFromClipboard()
{
    const QMimeData *mimedata = QGuiApplication::clipboard()->mimeData();
    QList<QGraphicsItem*> items;
    if (mimedata->hasFormat("application/beamerpresenter"))
    {
        const QByteArray data = mimedata->data("application/beamerpresenter");
        QDataStream stream(data);
        stream >> items;
        items.removeAll(nullptr);
    }
    else if (mimedata->hasFormat("image/svg+xml"))
    {
        QXmlStreamReader reader(mimedata->data("image/svg+xml"));
        while (!reader.atEnd() && (reader.readNext() != QXmlStreamReader::StartElement || reader.name().toUtf8() != "svg")) {}
        while (reader.readNextStartElement())
        {
            debug_msg(DebugDrawing, reader.name());
            if (reader.name().toUtf8() == "g")
            {
                const QPointF pos(reader.attributes().value("x", "0.").toDouble(), reader.attributes().value("y", "0.").toDouble());
                QTransform transform;
                QString transform_string = reader.attributes().value("transform").toString();
                if (transform_string.length() >= 19 && transform_string.startsWith("matrix(") && transform_string.endsWith(")"))
                {
                    // TODO: this is inefficient
                    transform_string.remove("matrix(");
                    transform_string.remove(")");
                    const QStringList list = transform_string.trimmed().replace(" ", ",").split(",");
                    if (list.length() == 6)
                        transform.setMatrix(list[0].toDouble(), list[1].toDouble(), 0., list[2].toDouble(), list[3].toDouble(), 0., list[4].toDouble(), list[5].toDouble(), 1.);
                }
                while (reader.readNextStartElement() && reader.name().toUtf8() != "path" && !reader.isEndElement())
                    reader.skipCurrentElement();
                debug_msg(DebugDrawing, reader.name());
                if (reader.name().toUtf8() != "path")
                    continue;

                QString coordinates = reader.attributes().value("d").toString();
                coordinates.replace(",", " ");
                // TODO: currently all coordinates are interpreted as absolute coordinates.
                coordinates.remove("m");
                coordinates.remove("l");
                coordinates.remove("z");
                coordinates.remove("L");
                coordinates.remove("M");
                QRegularExpression re;
                re.setPattern(R"(\s\s+)");
                coordinates.replace(re, " ");
                coordinates = coordinates.trimmed();
                QString style = reader.attributes().value("style").toString();
                QColor color = Qt::black;
                qreal width = 1.;
                re.setPattern("stroke:(#[0-9a-fA-F]+)");
                if (style.contains("stroke:"))
                    color = QColor(re.match(style).captured(1));
                re.setPattern("stroke-width:([0-9.]+)");
                if (style.contains("stroke-width:"))
                    width = re.match(style).captured(1).toDouble();
                DrawTool tool(Tool::BasicSelectionTool, 0, QPen(color, width));
                BasicGraphicsPath *path = new BasicGraphicsPath(tool, coordinates);
                path->setPos(pos);
                path->setPos(sceneRect().center());
                path->setTransform(transform);
                items.append(path);
                if (!reader.isEndElement())
                    reader.skipCurrentElement();
            }
            else if (reader.name().toUtf8() == "text")
            {
                // TODO
            }
            if (!reader.isEndElement())
                reader.skipCurrentElement();
        }
        reader.clear();
    }
    if (items.isEmpty())
        return;
    clearSelection();
    for (const auto item : items)
    {
        if (item->scene() != this)
            addItem(item);
        item->show();
        item->setSelected(true);
    }
    emit sendAddPaths(page | page_part, items);
}

void SlideScene::selectionToForeground() const
{
    const QList<QGraphicsItem*> selection = selectedItems();
    // TODO: implement this
}

void SlideScene::toolChanged(const Tool *tool) noexcept
{
    if (tool->tool() & Tool::AnyDrawTool)
    {
        const DrawTool *draw_tool = static_cast<const DrawTool*>(tool);
        PathContainer::DrawHistoryStep *step = new PathContainer::DrawHistoryStep;
        for (auto item : selectedItems())
        {
            switch (item->type())
            {
            case BasicGraphicsPath::Type:
            case FullGraphicsPath::Type:
            {
                auto path = static_cast<AbstractGraphicsPath*>(item);
                const QRgb color_diff = path->getTool().color().rgba() ^ draw_tool->color().rgba();
                if (color_diff)
                    step->colorChanges[path] = color_diff;
                if (draw_tool->width() != path->getTool().width())
                    step->widthChanges[path] = draw_tool->width() / path->getTool().width();
                path->changeTool(*draw_tool);
                path->update();
                break;
            }
            }
        }
        if (!step->colorChanges.isEmpty() || !step->widthChanges.isEmpty())
            emit sendHistoryStep(page | page_part, step);
        else
            delete step;
    }
}

void SlideScene::colorChanged(const QColor &color) noexcept
{
    PathContainer::DrawHistoryStep *step = new PathContainer::DrawHistoryStep;
    for (auto item : selectedItems())
    {
        switch (item->type())
        {
        case BasicGraphicsPath::Type:
        case FullGraphicsPath::Type:
        {
            auto path = static_cast<AbstractGraphicsPath*>(item);
            if (path->getTool().color() != color)
            {
                step->colorChanges[item] = path->getTool().color().rgba() ^ color.rgba();
                DrawTool tool = path->getTool();
                tool.setColor(color);
                path->changeTool(tool);
                path->update();
            }
            break;
        }
        case TextGraphicsItem::Type:
        {
            auto text = static_cast<TextGraphicsItem*>(item);
            step->colorChanges[item] = text->defaultTextColor().rgba() ^ color.rgba();
            text->setDefaultTextColor(color);
            break;
        }
        }
    }
    if (!step->colorChanges.isEmpty())
        emit sendHistoryStep(page | page_part, step);
    else
        delete step;
}

void SlideScene::widthChanged(const qreal width) noexcept
{
    PathContainer::DrawHistoryStep *step = new PathContainer::DrawHistoryStep;
    for (auto item : selectedItems())
    {
        switch (item->type())
        {
        case BasicGraphicsPath::Type:
        case FullGraphicsPath::Type:
        {
            auto path = static_cast<AbstractGraphicsPath*>(item);
            if (path->getTool().width() != width)
            {
                step->colorChanges[item] = width / path->getTool().width();
                DrawTool tool = path->getTool();
                tool.setWidth(width);
                path->changeTool(tool);
                path->update();
            }
            break;
        }
        }
    }
    if (!step->widthChanges.isEmpty())
        emit sendHistoryStep(page | page_part, step);
    else
        delete step;
}
