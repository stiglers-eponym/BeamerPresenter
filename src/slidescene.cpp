#include "src/slidescene.h"
#include "src/slideview.h"
#include "src/pdfmaster.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/flexgraphicslineitem.h"
#include "src/drawing/pixmapgraphicsitem.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pathcontainer.h"
#include "src/preferences.h"
#include "src/names.h"

SlideScene::SlideScene(const PdfMaster *master, const PagePart part, QObject *parent) :
    QGraphicsScene(parent),
    pageItem(new PixmapGraphicsItem(sceneRect())),
    master(master),
    page_part(part)
{
    connect(this, &SlideScene::sendNewPath, master, &PdfMaster::receiveNewPath);
    connect(this, &SlideScene::requestPathContainer, master, &PdfMaster::requestPathContainer, Qt::DirectConnection);
    pageItem->setZValue(-1e2);
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
        device = Tool::TouchInput | Tool::StartEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
        for (const auto &point : touchevent->touchPoints())
            pos.append(point.scenePos());
        break;
    }
    case QEvent::TouchUpdate:
    {
        device = Tool::TouchInput | Tool::UpdateEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
        for (const auto &point : touchevent->touchPoints())
            pos.append(point.scenePos());
        break;
    }
    case QEvent::TouchEnd:
    {
        device = Tool::TouchInput | Tool::StopEvent;
        const auto touchevent = static_cast<QTouchEvent*>(event);
        if (touchevent->touchPoints().size() > 0)
        {
            for (const auto &point : touchevent->touchPoints())
                pos.append(point.scenePos());
            start_pos = touchevent->touchPoints().constFirst().startScenePos();
        }
        break;
    }
    case QEvent::TouchCancel:
        device = Tool::TouchInput | Tool::CancelEvent;
        break;
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

    debug_verbose(DebugDrawing) << "Handling event" << tool->tool() << tool->device() << device;
    if (tool->tool() & Tool::AnyDrawTool)
    {
        switch (device & Tool::AnyEvent)
        {
        case Tool::UpdateEvent:
            stepInputEvent(static_cast<const DrawTool*>(tool), pos.constFirst(), pressure);
            break;
        case Tool::StartEvent:
            startInputEvent(static_cast<const DrawTool*>(tool), pos.constFirst(), pressure);
            break;
        case Tool::StopEvent:
            stopInputEvent(static_cast<const DrawTool*>(tool), pos.isEmpty() ? QPointF() : pos.constFirst());
            break;
        case Tool::CancelEvent:
            if (stopInputEvent(static_cast<const DrawTool*>(tool), QPointF()))
            {
                PathContainer *container;
                emit requestPathContainer(&container, page | page_part);
                if (container)
                    container->undo(this);
                break;
            }
        }
    }
    else if (tool->tool() & Tool::AnyPointingTool)
    {
        PointingTool *ptool = static_cast<PointingTool*>(tool);
        QRectF point_rect = QRectF({0,0}, ptool->size()*QSize(2,2));
        for (auto point : ptool->pos())
        {
            point_rect.moveCenter(point);
            invalidate(point_rect, QGraphicsScene::ForegroundLayer);
        }
        if ((device & Tool::AnyEvent) == Tool::StopEvent)
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
    }
    else if (tool->tool() & Tool::AnySelectionTool)
    {
        // TODO
    }
    else if (tool->tool() == Tool::TextInputTool && (device & Tool::AnyEvent) == Tool::StopEvent && pos.size() == 1)
    {
        debug_msg(DebugDrawing) << "Trying to start writing text" << (device & Tool::AnyDevice) << focusItem();
        for (auto item : static_cast<const QList<QGraphicsItem*>>(items(pos.constFirst())))
        {
            if (item->type() == TextGraphicsItem::Type)
            {
                setFocusItem(item);
                return;
            }
        }
        TextGraphicsItem *item = new TextGraphicsItem();
        item->setTextInteractionFlags(Qt::TextEditorInteraction);
        item->setFont(QFont(static_cast<const TextTool*>(tool)->font()));
        item->setDefaultTextColor(static_cast<const TextTool*>(tool)->color());
        addItem(item);
        item->show();
        item->setPos(pos.constFirst());
        emit sendNewPath(page | page_part, item);
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
        if (container)
            connect(item, &TextGraphicsItem::deleteMe, container, &PathContainer::deleteEmptyItem);
        setFocusItem(item);
    }
    else if ((device & Tool::AnyEvent) == Tool::StopEvent && pos.size() == 1)
        noToolClicked(pos.constFirst(), start_pos);
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
    case PauseMedia:
        pauseMedia();
        break;
    case PlayMedia:
        playMedia();
        break;
    case PlayPauseMedia:
        playPauseMedia();
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
    debug_msg(DebugPageChange) << "scene" << this << "navigates to" << newpage << "as" << newscene;
    pauseMedia();
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
        PdfDocument::SlideTransition transition;
        if (newpage > page)
            transition = master->transition(newpage);
        else
        {
            transition = master->transition(page);
            transition.invert();
        }
        if (transition.type)
        {
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
        addItem(pageItem);
        loadMedia(page);
        if (slide_flags & ShowDrawings)
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
    emit finishTransition();
}

void SlideScene::loadMedia(const int page)
{
    if (!(slide_flags & LoadMedia))
        return;
    QList<PdfDocument::MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : qAsConst(*list))
    {
        switch (annotation.type)
        {
        case PdfDocument::MediaAnnotation::VideoAnnotation:
        {
            debug_msg(DebugMedia) << "loading video" << annotation.file << annotation.rect;
            VideoItem &item = getVideoItem(annotation, page);
            item.item->setSize(item.annotation.rect.size());
            item.item->setPos(item.annotation.rect.topLeft());
            item.item->show();
            addItem(item.item);
            if (slide_flags & AutoplayVideo)
                item.player->play();
            break;
        }
        case PdfDocument::MediaAnnotation::AudioAnnotation:
        {
            qWarning() << "Sound annotation: not implemented yet";
            break;
        }
        case PdfDocument::MediaAnnotation::InvalidAnnotation:
            break;
        }
    }
}

void SlideScene::postRendering()
{
    pageItem->clearOld();
    int newpage = page + 1;
    if (shift & AnyOverlay)
        newpage = master->getDocument()->overlaysShifted(page, 1 | (shift & AnyOverlay));
    if (slide_flags & CacheVideos)
        cacheMedia(newpage);
}

void SlideScene::cacheMedia(const int page)
{
    QList<PdfDocument::MediaAnnotation> *list = master->getDocument()->annotations(page);
    if (!list)
        return;
    for (const auto &annotation : qAsConst(*list))
    {
        switch (annotation.type)
        {
        case PdfDocument::MediaAnnotation::VideoAnnotation:
            debug_msg(DebugMedia) << "try to cache video" << annotation.file << annotation.rect;
            getVideoItem(annotation, page);
            break;
        case PdfDocument::MediaAnnotation::AudioAnnotation:
            qWarning() << "Sound annotation: not implemented yet";
            break;
        case PdfDocument::MediaAnnotation::InvalidAnnotation:
            break;
        }
    }
}

SlideScene::VideoItem &SlideScene::getVideoItem(const PdfDocument::MediaAnnotation &annotation, const int page)
{
    for (auto &videoitem : videoItems)
    {
        if (videoitem.annotation == annotation && videoitem.item)
        {
            debug_msg(DebugMedia) << "Found video in cache" << annotation.file << annotation.rect;
            videoitem.pages.insert(page);
            return videoitem;
        }
    }
    debug_msg(DebugMedia) << "Loading new video" << annotation.file << annotation.rect;
    QMediaPlayer *player = new QMediaPlayer(this);
    player->setMuted(slide_flags & Mute);
    QMediaPlaylist *playlist = new QMediaPlaylist(player);
    QGraphicsVideoItem *item = new QGraphicsVideoItem;
    connect(item, &QGraphicsVideoItem::destroyed, player, &QMediaPlayer::deleteLater);
    // Ugly fix to cache videos: show invisible video pixel
    item->setSize({1,1});
    item->setPos(sceneRect().bottomRight());
    addItem(item);
    item->show();
    player->setVideoOutput(item);
    playlist->addMedia(annotation.file);
    switch (annotation.mode)
    {
    case PdfDocument::MediaAnnotation::Repeat:
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        break;
    case PdfDocument::MediaAnnotation::Palindrome:
        qWarning() << "Palindrome video: not implemented (yet)";
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        break;
    case PdfDocument::MediaAnnotation::Once:
    case PdfDocument::MediaAnnotation::Open:
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
        break;
    default:
        break;
    }
    player->setPlaylist(playlist);
    videoItems.append({annotation, item, player, {page}});
    return videoItems.last();
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
    debug_msg(DebugTransitions) << "transition:" << transition.type << transition.duration << transition.angle << transition.properties;
    QList<QGraphicsItem*> list = items();
    while (!list.isEmpty())
        removeItem(list.takeLast());
    pageItem->setOpacity(1.);
    addItem(pageItem);
    loadMedia(page);
    if (slide_flags & ShowDrawings)
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
    PathContainer *paths;
    emit requestPathContainer(&paths, page | page_part);
    if (paths)
    {
        const auto end = paths->cend();
        for (auto it = paths->cbegin(); it != end; ++it)
            if (*it)
                addItem(*it);
    }
    loadMedia(page);
    invalidate();
    emit finishTransition();
}

void SlideScene::tabletPress(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                (event->pressure() > 0 ? tablet_device_to_input_device.value(event->pointerType()) : Tool::TabletHover) | Tool::StartEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::tabletMove(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                (event->pressure() > 0 ? tablet_device_to_input_device.value(event->pointerType()) : Tool::TabletHover) | Tool::UpdateEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::tabletRelease(const QPointF &pos, const QTabletEvent *event)
{
    handleEvents(
                tablet_device_to_input_device.value(event->pointerType()) | Tool::StopEvent,
                {pos},
                QPointF(),
                event->pressure()
            );
}

void SlideScene::startInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure)
{
    if (!tool || !(tool->tool() & Tool::AnyDrawTool) || !(slide_flags & ShowDrawings))
        return;
    debug_verbose(DebugDrawing) << "Start input event" << tool->tool() << tool->device() << tool << pressure;
    stopDrawing();
    switch (tool->tool())
    {
    case Tool::Pen:
    case Tool::FixedWidthPen:
    case Tool::Highlighter:
        if (currentItemCollection || currentPath)
            break;
        currentItemCollection = new QGraphicsItemGroup();
        addItem(currentItemCollection);
        currentItemCollection->show();
        if (tool->tool() == Tool::Pen && (tool->device() & Tool::PressureSensitiveDevices))
            currentPath = new FullGraphicsPath(*tool, pos, pressure);
        else
            currentPath = new BasicGraphicsPath(*tool, pos);
        addItem(currentPath);
        currentPath->hide();
        break;
    case Tool::Eraser:
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

void SlideScene::stepInputEvent(const DrawTool *tool, const QPointF &pos, const float pressure)
{
    if (pressure <= 0 || !tool || !(slide_flags & ShowDrawings))
        return;
    debug_verbose(DebugDrawing) << "Step input event" << tool->tool() << tool->device() << tool << pressure;
    switch (tool->tool())
    {
    case Tool::Pen:
    case Tool::FixedWidthPen:
    case Tool::Highlighter:
        if (currentPath && currentItemCollection && *tool == currentPath->getTool())
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
    case Tool::Eraser:
    {
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
        if (container)
            container->eraserMicroStep(pos, tool->width());
        break;
    }
    default:
        break;
    }
}

bool SlideScene::stopInputEvent(const DrawTool *tool, const QPointF &pos)
{
    if (!tool || !(slide_flags & ShowDrawings))
        return false;
    debug_verbose(DebugDrawing) << "Stop input event" << tool->tool() << tool->device() << tool;
    const bool changes = currentPath && currentPath->size() > 1;
    stopDrawing();
    switch (tool->tool())
    {
    case Tool::Pen:
    case Tool::FixedWidthPen:
    case Tool::Highlighter:
        if (changes)
        {
            invalidate({QRect()}, QGraphicsScene::ItemLayer);
            return true;
        }
        break;
    case Tool::Eraser:
    {
        PathContainer *container;
        emit requestPathContainer(&container, page | page_part);
        if (container && container->applyMicroStep())
        {
            emit newUnsavedDrawings();
            return true;
        }
        break;
    }
    case Tool::NoTool:
        noToolClicked(pos);
        break;
    default:
        break;
    }
    return false;
}

void SlideScene::noToolClicked(const QPointF &pos, const QPointF &startpos)
{
    debug_verbose(DebugMedia) << "Clicked without tool" << pos << startpos;
    // Try to handle multimedia annotation.
    for (auto &item : videoItems)
    {
        if (item.pages.contains((page &~NotFullPage)) && item.item->boundingRect().contains(pos))
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

void SlideScene::createSliders() const
{
    for (auto &item : videoItems)
    {
        if (item.pages.contains((page &~NotFullPage)))
        {
            for (const auto view : static_cast<const QList<QGraphicsView*>>(views()))
                static_cast<SlideView*>(view)->addMediaSlider(item);
        }
    }
}

void SlideScene::playMedia() const
{
    for (auto &item : videoItems)
    {
        if (item.pages.contains((page &~NotFullPage)))
            item.player->play();
    }
}

void SlideScene::pauseMedia() const
{
    for (auto &item : videoItems)
    {
        if (item.pages.contains((page &~NotFullPage)))
            item.player->pause();
    }
}

void SlideScene::playPauseMedia() const
{
    for (auto &item : videoItems)
    {
        if (item.pages.contains((page &~NotFullPage)) && item.player->state() == QMediaPlayer::PlayingState)
        {
            pauseMedia();
            return;
        }
    }
    playMedia();
}
