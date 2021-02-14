#include "src/drawing/pathcontainer.h"

PathContainer::~PathContainer()
{
    truncateHistory();
    clearHistory();
    // This is dangerous: check which paths are owned by QGraphicsScene.
    qDeleteAll(paths);
    paths.clear();
}

bool PathContainer::undo(QGraphicsScene *scene)
{
    // Check whether a further entry in history exists.
    if (inHistory < 0 || history.length() - inHistory < 1)
        return false;

    // Mark that we moved back in history.
    inHistory++;

    // First remove newly created items.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &removeItems = history[history.length()-inHistory]->createdItems;
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = removeItems.constEnd(); it-- != removeItems.constBegin();)
    {
        paths.removeAt(it.key());
        if ((*it)->scene())
            (*it)->scene()->removeItem(it.value());
    }

    // Restore old items.
    // Get the old items from history.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->deletedItems;
    for (auto it = oldItems.constBegin(); it != oldItems.constEnd(); ++it)
    {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene)
        {
            scene->addItem(it.value());
            if (it.key() + 1 < paths.length())
                it.value()->stackBefore(paths[it.key() + 1]);
        }
    }

    return true;
}

bool PathContainer::redo(QGraphicsScene *scene)
{
    // First check whether there is something to redo in history.
    if (inHistory < 1)
        return false;

    // First remove items which were deleted in this step.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->deletedItems;
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = oldItems.constEnd(); it-- != oldItems.constBegin();)
    {
        paths.removeAt(it.key());
        if (it.value()->scene())
            it.value()->scene()->removeItem(it.value());
    }

    // Restore newly created items.
    // Get the new items from history.
    const QMap<int, QGraphicsItem*> &newItems = history[history.length()-inHistory]->createdItems;
    for (auto it = newItems.constBegin(); it != newItems.constEnd(); ++it)
    {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene)
        {
            scene->addItem(it.value());
            if (it.key() + 1 < paths.length())
                it.value()->stackBefore(paths[it.key() + 1]);
        }
    }

    // Move forward in history.
    inHistory--;
    return true;
}

void PathContainer::truncateHistory()
{
    if (inHistory == -1)
        applyMicroStep();
    else if (inHistory == -2)
        inHistory = 0;
    else
    {
        // Clean up all "redo" options:
        // Delete the last <inHistory> history entries.
        while (inHistory > 0)
        {
            // Take the last step from history (removes it from history).
            DrawHistoryStep *step = history.takeLast();
            // Delete all future objects in this step. These objects are not visible.
            qDeleteAll(step->createdItems);
            step->createdItems.clear();
            // Delete the step. The past objects of the step are untouched, since
            // they are still owned by other history steps or by this.
            delete step;
            --inHistory;
        }
    }
}

void PathContainer::clearHistory(int n)
{
    if (inHistory == -2)
        return;
    if (inHistory == -1)
        applyMicroStep();

    // Negative values of n don't make any sense and are interpreted as 0.
    if (n < 0)
        n = 0;

    // Delete the first entries in history until
    // history.length() - inHistory <= n .
    for (int i = history.length() - inHistory; i > n; i--)
    {
        // Take the first step from history (removes it from history).
        DrawHistoryStep *step = history.takeFirst();
        // Delete all past objects in this step. These objects are not visible.
        // TODO: Does this delete the correct part?
        qDeleteAll(step->deletedItems);
        step->deletedItems.clear();
        // Delete the step. The future objects of the step are untouched, since
        // they are still owned by other history steps or by this.
        delete step;
    }
}

void PathContainer::clearPaths()
{
    truncateHistory();
    // Create a new history step.
    DrawHistoryStep *step = new DrawHistoryStep();
    // Append all paths to the history step.
    // If scene != NULL, additionally remove the items from the scene.
    auto it = paths.cbegin();
    for (int i=0; i<paths.length(); i++, ++it)
    {
        step->deletedItems[i] = *it;
        if (*it && (*it)->scene())
            (*it)->scene()->removeItem(*it);
    }
    // Add the scene to history.
    history.append(step);
    // All paths have been added to history. paths can be cleared.
    paths.clear();
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}

void PathContainer::append(QGraphicsItem *item)
{
    // Remove all "redo" options.
    truncateHistory();
    // Create new history step which adds item.
    const auto step = new DrawHistoryStep();
    step->createdItems[paths.length()] = item;
    history.append(step);
    // Add item to paths.
    paths.append(item);
    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}

void PathContainer::startMicroStep()
{
    // Remove all "redo" options.
    truncateHistory();
    // Create new, empty history step.
    history.append(new DrawHistoryStep());
    inHistory = -1;
}

void PathContainer::eraserMicroStep(const QPointF &pos, const qreal size)
{
    if (inHistory != -1)
    {
        qCritical() << "Tried microstep, but inHistory == " << inHistory;
        return;
    }

    // Iterate over all paths and check whether they intersect with pos.
    QList<QGraphicsItem*>::iterator path_it = paths.begin();
    for (int i=0; path_it != paths.end(); ++path_it, i++)
    {
        // Check if pos lies within the path's bounding rect (plus extra
        // margins from the eraser size).
        if (*path_it && (*path_it)->boundingRect().marginsAdded(QMargins(size, size, size, size)).contains(pos))
        {
            QGraphicsScene *scene = (*path_it)->scene();
            switch ((*path_it)->type())
            {
            case AbstractGraphicsPath::Type:
            case FullGraphicsPath::Type:
            case BasicGraphicsPath::Type:
            {
                AbstractGraphicsPath *path = static_cast<AbstractGraphicsPath*>(*path_it);
                // Apply eraser to path. Get a list of paths obtained by splitting
                // path using the eraser.
                QList<AbstractGraphicsPath*> list = path->splitErase(pos, size);
                // If list is empty, the path was completely erased.
                if (list.isEmpty())
                {
                    // Mark in history step that this path is deleted.
                    history.last()->deletedItems[i] = path;
                    // Hide the path, remove it from scene (if possible).
                    if (scene)
                        scene->removeItem(path);
                    else
                        path->hide();
                    // Remove path from paths by settings paths[i] = NULL.
                    *path_it = NULL;
                }
                // If nothing should change (the path is not affected by erasing),
                // the list will only contain a NULL. In this case we do nothing.
                // In all other cases:
                else if (list.first())
                {
                    // Path has changed or was split into multiple paths by erasing.
                    // Replace path by a QGraphicsItemGroup containing all new paths
                    // created from this path.

                    // First mark this path as removed in history.
                    history.last()->deletedItems[i] = path;
                    // Create the QGraphicsItemGroup.
                    QGraphicsItemGroup *group = new QGraphicsItemGroup();
                    // Add all paths in list (which were obtained by erasing in path)
                    // to group.
                    for (const auto item : qAsConst(list))
                    {
                        group->addToGroup(item);
                        item->show(); // TODO: necessary?
                    }
                    // Replace path by group in scene (if possible).
                    if (scene)
                    {
                        scene->addItem(group);
                        group->stackBefore(path);
                        scene->removeItem(path);
                    }
                    else
                        path->hide();
                    // Replace path by group in paths.
                    *path_it = group;
                }
                break;
            }
            case QGraphicsItemGroup::Type:
            {
                // Here a path has already been split into a QGraphicsItemGroup by erasing.
                // Apply eraser again to all items of the group.
                // Within the group, stacking order is irrelevant since all items were
                // created from the same path by erasing.
                QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(*path_it);
                for (const auto child : static_cast<const QList<QGraphicsItem*>>(group->childItems()))
                {
                    // All items in the group should be paths. But we better check again.
                    if (child && (child->type() == FullGraphicsPath::Type || child->type() == BasicGraphicsPath::Type))
                    {
                        // Apply eraser to child.
                        const auto list = static_cast<AbstractGraphicsPath*>(child)->splitErase(pos, size);
                        // Again, if list.first() == NULL, we should do nothing
                        // because the eraser did not hit the path.
                        if (list.isEmpty() || list.first())
                        {
                            for (auto item : list)
                                group->addToGroup(item);
                            group->removeFromGroup(child);
                            if (scene)
                                scene->removeItem(child);
                            // This item is not stored in any history and can be deleted.
                            delete child; // TODO: Check if this breaks something.
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

bool PathContainer::applyMicroStep()
{
    if (inHistory != -1)
    {
        qCritical() << "Should apply micro step, but inHistory ==" << inHistory;
        inHistory = 0;
        return true;
    }

    // 1. Fix the history step.
    // In eraserMicroStep() only deletions are added to history.last() while
    // creating new paths is not marked in history. This is done here.
    // At each index i in oldItems.keys(), paths[i] has either been removed
    // or replaced by a QGraphicsItemGroup* containing new paths.
    // Here we add these new paths to history.last().
    const QMap<int, QGraphicsItem*> &oldItems = history.last()->deletedItems;
    if (oldItems.isEmpty())
    {
        inHistory = 0;
        history.pop_back();
        return false;
    }

    int shift = 0;
    for (auto it = oldItems.cbegin(); it != oldItems.cend(); ++it, shift--)
    {
        // item should be either NULL (if this path has been erased completely)
        // or a QGraphicsItemGroup* (if this path has been erased partially).
        QGraphicsItem *item = paths.value(it.key());
        if (item && item->type() == QGraphicsItemGroup::Type)
        {
            // If item is a QGraphicsItemGroup*: Add all its child paths to
            // history.last().
            QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(item);
            for (const auto child : static_cast<const QList<QGraphicsItem*>>(group->childItems()))
            {
                // The index shift "shift" is given by #(new items) - #(delted items)
                // which lie before it.key() in paths.
                history.last()->createdItems[it.key() + shift++] = child;
                group->removeFromGroup(child);
                child->stackBefore(group);
            }
            if (group->scene())
                group->scene()->removeItem(group);
            delete group;
        }
    }

    // 2. Remove all deleted items from paths.
    // The removed items have already been deleted.
    {
        auto key = oldItems.keyEnd();
        while (key != oldItems.keyBegin())
            paths.removeAt(*--key);
    }

    // 3. Add newly created items to path.
    // Get the new items from history (as prepared in step 1.)
    const QMap<int, QGraphicsItem*> &newItems = history.last()->createdItems;
    for (auto it = newItems.cbegin(); it != newItems.cend(); ++it)
        paths.insert(it.key(), it.value());

    // Move forward in history.
    inHistory = 0;

    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
    return true;
}

PathContainer *PathContainer::copy() const noexcept
{
    PathContainer *container = new PathContainer();
    container->inHistory = -2;
    for (const auto path : qAsConst(paths))
    {
        switch (path->type())
        {
        case QGraphicsTextItem::Type:
        {
            QGraphicsTextItem *olditem = static_cast<QGraphicsTextItem*>(path);
            QGraphicsTextItem *newitem = new QGraphicsTextItem();
            newitem->setPos(path->pos());
            newitem->setFont(olditem->font());
            newitem->setHtml(olditem->toHtml());
            newitem->setDefaultTextColor(olditem->defaultTextColor());
            container->paths.append(newitem);
            break;
        }
        case FullGraphicsPath::Type:
            container->paths.append(new FullGraphicsPath(static_cast<FullGraphicsPath*>(path), 0, -1));
            break;
        case BasicGraphicsPath::Type:
            container->paths.append(new BasicGraphicsPath(static_cast<BasicGraphicsPath*>(path), 0, -1));
            break;
        case QGraphicsLineItem::Type:
            container->paths.append(new QGraphicsLineItem(static_cast<QGraphicsLineItem*>(path)->line()));
            break;
        }
    }
    return container;
}

void PathContainer::writeXml(QXmlStreamWriter &writer) const
{
    for (const auto path : qAsConst(paths))
    {
        switch (path->type())
        {
        case QGraphicsTextItem::Type:
        {
            const QGraphicsTextItem *item = static_cast<QGraphicsTextItem*>(path);
            writer.writeStartElement("text");
            writer.writeAttribute("font", QFontInfo(item->font()).family());
            writer.writeAttribute("size", QString::number(item->font().pointSizeF()));
            writer.writeAttribute("color", color_to_rgba(item->defaultTextColor()).toLower());
            writer.writeAttribute("x", QString::number(item->x()));
            writer.writeAttribute("y", QString::number(item->y()));
            writer.writeCharacters(item->toPlainText());
            writer.writeEndElement();
            break;
        }
        case FullGraphicsPath::Type:
        case BasicGraphicsPath::Type:
        {
            const AbstractGraphicsPath *item = static_cast<AbstractGraphicsPath*>(path);
            const DrawTool &tool = item->getTool();
            writer.writeStartElement("stroke");
            writer.writeAttribute("tool", xournal_tool_names.value(tool.tool()));
            writer.writeAttribute("color", color_to_rgba(tool.color()).toLower());
            writer.writeAttribute("width", item->stringWidth());
            writer.writeCharacters(item->stringCoordinates());
            writer.writeEndElement();
            break;
        }
        }
    }
}

AbstractGraphicsPath *loadPath(QXmlStreamReader &reader)
{
    BasicTool basic_tool = string_to_tool.value(reader.attributes().value("tool").toString());
    if (!(basic_tool & AnyDrawTool))
        return NULL;
    const QString width_str = reader.attributes().value("width").toString();
    if (basic_tool == Pen && !width_str.contains(' '))
        basic_tool = FixedWidthPen;
    QPen pen(
                rgba_to_color(reader.attributes().value("color").toString()),
                basic_tool == Pen ? 1. : width_str.toDouble(),
                Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin
                );
    if (pen.widthF() <= 0)
        pen.setWidthF(1.);
    DrawTool *tool = new DrawTool(basic_tool, AnyNormalDevice, pen, basic_tool == Highlighter ? QPainter::CompositionMode_Darken : QPainter::CompositionMode_SourceOver);
    if (basic_tool == Pen)
        return new FullGraphicsPath(*tool, reader.readElementText(), width_str);
    else
        return new BasicGraphicsPath(*tool, reader.readElementText());
}

QGraphicsTextItem *loadTextItem(QXmlStreamReader &reader)
{
    QGraphicsTextItem *item = new QGraphicsTextItem();
    QPointF pos;
    pos.setX(reader.attributes().value("x").toDouble());
    pos.setY(reader.attributes().value("y").toDouble());
    item->setPos(pos);
    QFont font(reader.attributes().value("font").toString());
    font.setPointSizeF(reader.attributes().value("size").toDouble());
    item->setFont(font);
    const QString text = reader.readElementText();
    if (text.isEmpty())
    {
        delete item;
        return NULL;
    }
    item->setPlainText(text);
    item->setDefaultTextColor(rgba_to_color(reader.attributes().value("color").toString()));
    return item;
}

void PathContainer::loadDrawings(QXmlStreamReader &reader)
{
    QGraphicsItem *item;
    while (reader.readNextStartElement())
    {
        if (reader.name() == "stroke")
            item = loadPath(reader);
        else if (reader.name() == "text")
            item = loadTextItem(reader);
        else
            reader.skipCurrentElement();
        if (item)
            paths.append(item);
    }
}

void PathContainer::loadDrawings(QXmlStreamReader &reader, PathContainer *left, PathContainer *right, const qreal page_half)
{
    while (reader.readNextStartElement())
    {
        if (reader.name() == "stroke")
        {
            AbstractGraphicsPath *path = loadPath(reader);
            if (!path)
                continue;
            if (path->firstPoint().x() > page_half)
                right->paths.append(path);
            else
                left->paths.append(path);
        }
        else if (reader.name() == "text")
        {
            QGraphicsTextItem *item = loadTextItem(reader);
            if (!item)
                continue;
            if (item->pos().x() > page_half)
                right->paths.append(item);
            else
                left->paths.append(item);
        }
        else
            reader.skipCurrentElement();
    }
}

QRectF PathContainer::boundingBox() const noexcept
{
    QRectF rect;
    for (const auto path : qAsConst(paths))
        rect = rect.united(path->sceneBoundingRect());
    debug_msg(DebugDrawing) << "boudding rect:" << rect;
    return rect;
}
