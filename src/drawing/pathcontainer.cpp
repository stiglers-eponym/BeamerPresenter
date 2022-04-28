#include "src/drawing/pathcontainer.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/names.h"
#include "src/preferences.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>


PathContainer::~PathContainer()
{
    truncateHistory();
    clearHistory();
    // This is dangerous: check which paths are owned by QGraphicsScene.
    // TODO: This has at least once led to a segmentation fault when closing the program.
    while (!paths.isEmpty())
        delete paths.takeLast();
}

bool PathContainer::undo(QGraphicsScene *scene)
{
    // Check whether a further entry in history exists.
    if (inHistory < 0 || history.length() - inHistory < 1)
        return false;

    // Mark that we moved back in history.
    inHistory++;

    const DrawHistoryStep *step = history[history.length() - inHistory];

    // 1. Undo transformations.
    for (auto it = step->transformedItems.constBegin(); it != step->transformedItems.constEnd(); ++it)
        it.key()->setTransform(it->inverted(), true);

    // 2. Undo draw tool changes.
    for (auto it = step->drawToolChanges.constBegin(); it != step->drawToolChanges.constEnd(); ++it)
    {
        if (it.key()->type() != FullGraphicsPath::Type && it.key()->type() != BasicGraphicsPath::Type)
        {
            // this should never happen
            warn_msg("History of draw tool changes includes item of invalid type" << it.key()->type());
            continue;
        }
        auto path = static_cast<AbstractGraphicsPath*>(it.key());
        DrawTool tool = path->getTool();
        tool.setPen(it->old_pen);
        tool.brush() = it->old_brush;
        path->changeTool(tool);
        path->update();
    }

    // 3. Undo text tool changes.
    for (auto it = step->textPropertiesChanges.constBegin(); it != step->textPropertiesChanges.constEnd(); ++it)
    {
        if (it.key()->type() != TextGraphicsItem::Type)
        {
            // this should never happen
            warn_msg("History of text propery changes includes item of invalid type" << it.key()->type());
            continue;
        }
        auto text = static_cast<TextGraphicsItem*>(it.key());
        text->setFont(it->old_font);
        text->setDefaultTextColor(text->defaultTextColor().rgba() ^ it->color_diff);
        text->update();
    }

    // 4. Remove newly created items.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &removeItems = step->createdItems;
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = removeItems.constEnd(); it != removeItems.constBegin();)
    {
        paths.removeAt((--it).key());
        if ((*it)->scene())
        {
            (*it)->clearFocus();
            (*it)->scene()->removeItem(*it);
        }
    }

    // 5. Restore old items.
    // Get the old items from history.
    const QMap<int, QGraphicsItem*> &oldItems = step->deletedItems;
    for (auto it = oldItems.constBegin(); it != oldItems.constEnd(); ++it)
    {
        paths.insert(it.key(), *it);
        if (scene)
        {
            scene->addItem(*it);
            if (it.key() + 1 < paths.length())
                (*it)->stackBefore(paths[it.key() + 1]);
        }
        (*it)->show();
    }

    return true;
}

bool PathContainer::redo(QGraphicsScene *scene)
{
    // First check whether there is something to redo in history.
    if (inHistory < 1)
        return false;

    const DrawHistoryStep *step = history[history.length() - inHistory];
    // Move forward in history.
    inHistory--;

    // 1. First remove items which were deleted in this step.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &oldItems = step->deletedItems;
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = oldItems.constEnd(); it != oldItems.constBegin();)
    {
        paths.removeAt((--it).key());
        if ((*it)->scene())
        {
            (*it)->clearFocus();
            (*it)->scene()->removeItem(*it);
        }
    }

    // 2. Restore newly created items.
    // Get the new items from history.
    const QMap<int, QGraphicsItem*> &newItems = step->createdItems;
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

    // 3. Redo draw tool changes.
    for (auto it = step->drawToolChanges.constBegin(); it != step->drawToolChanges.constEnd(); ++it)
    {
        if (it.key()->type() != FullGraphicsPath::Type && it.key()->type() != BasicGraphicsPath::Type)
        {
            // this should never happen
            warn_msg("History of draw tool changes includes item of invalid type" << it.key()->type());
            continue;
        }
        auto path = static_cast<AbstractGraphicsPath*>(it.key());
        DrawTool tool = path->getTool();
        tool.setPen(it->new_pen);
        tool.brush() = it->new_brush;
        path->changeTool(tool);
        path->update();
    }

    // 4. Redo text tool changes.
    for (auto it = step->textPropertiesChanges.constBegin(); it != step->textPropertiesChanges.constEnd(); ++it)
    {
        if (it.key()->type() != TextGraphicsItem::Type)
        {
            // this should never happen
            warn_msg("History of text propery changes includes item of invalid type" << it.key()->type());
            continue;
        }
        auto text = static_cast<TextGraphicsItem*>(it.key());
        text->setFont(it->new_font);
        text->setDefaultTextColor(text->defaultTextColor().rgba() ^ it->color_diff);
        text->update();
    }

    // 5. Redo transformations.
    for (auto it = step->transformedItems.constBegin(); it != step->transformedItems.constEnd(); ++it)
        it.key()->setTransform(*it, true);

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
        step->deletedItems.insert(i, *it);
        if (*it && (*it)->scene())
        {
            (*it)->clearFocus();
            (*it)->scene()->removeItem(*it);
        }
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
    DrawHistoryStep *const step = new DrawHistoryStep();
    step->createdItems.insert(paths.length(), item);
    // Add item to paths.
    paths.append(item);
    // Add step to history.
    history.append(step);
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

void PathContainer::eraserMicroStep(const QPointF &scene_pos, const qreal size)
{
    if (inHistory != -1)
    {
        qCritical() << "Tried microstep, but inHistory == " << inHistory;
        return;
    }

    // Iterate over all paths and check whether they intersect with scene_pos.
    QList<QGraphicsItem*>::iterator path_it = paths.begin();
    for (int i=0; path_it != paths.end(); ++path_it, i++)
    {
        // Check if scene_pos lies within the path's bounding rect (plus extra
        // margins from the eraser size).
        if (
                *path_it
                && (*path_it)->sceneBoundingRect().marginsAdded(QMargins(size, size, size, size)).contains(scene_pos)
            )
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
                QList<AbstractGraphicsPath*> list = path->splitErase(scene_pos, size);
                // If list is empty, the path was completely erased.
                if (list.isEmpty())
                {
                    // Mark in history step that this path is deleted.
                    history.last()->deletedItems.insert(i, path);
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
                    history.last()->deletedItems.insert(i, path);
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
                        const auto list = static_cast<AbstractGraphicsPath*>(child)->splitErase(scene_pos, size);
                        // Again, if list.first() == NULL, we should do nothing
                        // because the eraser did not hit the path.
                        if (list.isEmpty() || list.first())
                        {
                            for (auto item : list)
                                group->addToGroup(item);
                            group->removeFromGroup(child);
                            if (scene)
                            {
                                child->clearFocus();
                                scene->removeItem(child);
                            }
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
                history.last()->createdItems.insert(it.key() + shift++, child);
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
    {
        paths.insert(it.key(), it.value());
        switch ((*it)->type())
        {
        case BasicGraphicsPath::Type:
        case FullGraphicsPath::Type:
            if (preferences()->global_flags & Preferences::FinalizeDrawnPaths)
                static_cast<AbstractGraphicsPath*>(*it)->finalize();
            break;
        }
    }

    // Move forward in history.
    inHistory = 0;

    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
    return true;
}

PathContainer *PathContainer::copy() const noexcept
{
    PathContainer *container = new PathContainer(parent());
    container->inHistory = -2;
    for (const auto path : qAsConst(paths))
    {
        switch (path->type())
        {
        case TextGraphicsItem::Type:
        {
            TextGraphicsItem *olditem = static_cast<TextGraphicsItem*>(path);
            if (!olditem->isEmpty())
            {
                TextGraphicsItem *newitem = olditem->clone();
                container->paths.append(newitem);
                connect(newitem, &TextGraphicsItem::removeMe, container, &PathContainer::removeItem);
                connect(newitem, &TextGraphicsItem::addMe, container, &PathContainer::addTextItem);
            }
            break;
        }
        case FullGraphicsPath::Type:
        case BasicGraphicsPath::Type:
            container->paths.append(static_cast<AbstractGraphicsPath*>(path)->copy());
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
        case TextGraphicsItem::Type:
        {
            const TextGraphicsItem *item = static_cast<TextGraphicsItem*>(path);
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
            switch (tool.tool())
            {
            case Tool::Pen:
            case Tool::FixedWidthPen:
                writer.writeAttribute("tool", "pen");
                break;
            case Tool::Highlighter:
                writer.writeAttribute("tool", "highlighter");
                break;
            default:
                break;
            }
            writer.writeAttribute("color", color_to_rgba(tool.color()).toLower());
            writer.writeAttribute("width", item->stringWidth());
            if (tool.pen().style() != Qt::SolidLine)
                writer.writeAttribute("style", string_to_pen_style.key(tool.pen().style()));
            if (tool.brush().style() != Qt::NoBrush)
            {
                // Compare brush and stroke color.
                const QColor &fill = tool.brush().color(), &stroke = tool.pen().color();
                if (fill.red() == stroke.red() && fill.green() == stroke.green() && fill.blue() == stroke.blue())
                {
                    // Write color in format that is compatible with Xournal++:
                    // Save only alpha relative to stroke color (as 8 bit int).
                    // avoid division by zero by tiny offset
                    float alpha = fill.alphaF() / (tool.pen().color().alphaF() + 1e-6);
                    writer.writeAttribute("fill", alpha >= 1 ? "255" : QString::number((int)(alpha*255+0.5)));
                }
                else
                {
                    // Write color to "brushcolor" attribute, which will be ignored by Xournal++
                    writer.writeAttribute("brushcolor", color_to_rgba(fill).toLower());
                }
                if (tool.brush().style() != Qt::SolidPattern)
                    writer.writeAttribute("brushstyle", string_to_brush_style.key(tool.brush().style()));
            }
            writer.writeCharacters(item->stringCoordinates());
            writer.writeEndElement();
            break;
        }
        }
    }
}

AbstractGraphicsPath *loadPath(QXmlStreamReader &reader)
{
    Tool::BasicTool basic_tool = string_to_tool.value(reader.attributes().value("tool").toString());
    if (!(basic_tool & Tool::AnyDrawTool))
        return NULL;
    const QString width_str = reader.attributes().value("width").toString();
    if (basic_tool == Tool::Pen && !width_str.contains(' '))
        basic_tool = Tool::FixedWidthPen;
    QPen pen(
                rgba_to_color(reader.attributes().value("color").toString()),
                basic_tool == Tool::Pen ? 1. : width_str.toDouble(),
                string_to_pen_style.value(reader.attributes().value("style").toString(), Qt::SolidLine),
                Qt::RoundCap,
                Qt::RoundJoin
                );
    if (pen.widthF() <= 0)
        pen.setWidthF(1.);
    // "fill" is the Xournal++ way of storing filling colors. However, it only
    // allows one to add transparency to the stroke color.
    int fill_xopp = reader.attributes().value("fill").toInt();
    // "brushcolor" is a BeamerPresenter extension of the Xournal++ file format
    Qt::BrushStyle brush_style = string_to_brush_style.value(reader.attributes().value("brushstyle").toString(), Qt::SolidPattern);
    QColor fill_color = rgba_to_color(reader.attributes().value("brushcolor").toString());
    if (!fill_color.isValid())
    {
        fill_color = pen.color();
        if (fill_xopp > 0 && fill_xopp < 256)
            fill_color.setAlphaF(fill_xopp*fill_color.alphaF()/255);
        else
            brush_style = Qt::NoBrush;
    }
    DrawTool *tool = new DrawTool(
                basic_tool,
                Tool::AnyNormalDevice,
                pen,
                QBrush(fill_color, brush_style),
                basic_tool == Tool::Highlighter ? QPainter::CompositionMode_Darken : QPainter::CompositionMode_SourceOver);
    if (basic_tool == Tool::Pen)
        return new FullGraphicsPath(*tool, reader.readElementText(), width_str);
    else
        return new BasicGraphicsPath(*tool, reader.readElementText());
}

TextGraphicsItem *loadTextItem(QXmlStreamReader &reader)
{
    TextGraphicsItem *item = new TextGraphicsItem();
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
        if (reader.name().toUtf8() == "stroke")
            item = loadPath(reader);
        else if (reader.name().toUtf8() == "text")
            item = loadTextItem(reader);
        else
        {
            reader.skipCurrentElement();
            continue;
        }
        if (item)
            paths.append(item);
    }
}

void PathContainer::loadDrawings(QXmlStreamReader &reader, PathContainer *left, PathContainer *right, const qreal page_half)
{
    while (reader.readNextStartElement())
    {
        if (reader.name().toUtf8() == "stroke")
        {
            AbstractGraphicsPath *path = loadPath(reader);
            if (!path)
                continue;
            if (path->firstPoint().x() > page_half)
                right->paths.append(path);
            else
                left->paths.append(path);
        }
        else if (reader.name().toUtf8() == "text")
        {
            TextGraphicsItem *item = loadTextItem(reader);
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
    return rect;
}

void PathContainer::replaceItem(QGraphicsItem *olditem, QGraphicsItem *newitem)
{
    const int index = olditem ? paths.indexOf(olditem) : -1;
    if (index < 0)
    {
        delete olditem;
        if (newitem)
        {
            if (!paths.contains(newitem))
                append(newitem);
            else if (inHistory == -2)
                inHistory = 0;
        }
        return;
    }
    if (newitem)
    {
        // Remove all "redo" options.
        truncateHistory();
        paths.insert(index, newitem);
        DrawHistoryStep *const step = new DrawHistoryStep();
        step->deletedItems.insert(index, olditem);
        step->createdItems.insert(index, newitem);
        history.append(step);
        // Remove item from it's scene (if it has one).
        if (olditem->scene())
        {
            olditem->clearFocus();
            olditem->scene()->removeItem(olditem);
        }
    }
    else
    {
        // Remove all "redo" options.
        truncateHistory();
        // Remove item from list of currently visible paths.
        paths.removeAt(index);
        DrawHistoryStep *const step = new DrawHistoryStep();
        step->deletedItems.insert(index, olditem);
        history.append(step);
        // Remove item from it's scene (if it has one).
        if (olditem->scene())
        {
            olditem->clearFocus();
            olditem->scene()->removeItem(olditem);
        }
    }
    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}

void PathContainer::addItems(const QList<QGraphicsItem*> &items)
{
    // Remove all "redo" options.
    truncateHistory();
    DrawHistoryStep *const step = new DrawHistoryStep();
    for (const auto item : items)
    {
        if (paths.contains(item))
        {
            // this should never happen
            warn_msg("Item already exists in history");
            continue;
        }
        step->createdItems.insert(paths.length(), item);
        paths.append(item);
    }
    history.append(step);
    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}

void PathContainer::removeItems(const QList<QGraphicsItem*> &items)
{
    // Remove all "redo" options.
    truncateHistory();
    DrawHistoryStep *const step = new DrawHistoryStep();
    int index;
    for (const auto item : items)
    {
        index = paths.indexOf(item);
        if (index < 0)
        {
            // this should never happen
            warn_msg("Item does not exist in history");
            continue;
        }
        step->deletedItems.insert(index, item);
    }
    for (auto it=step->deletedItems.cend(); it!=step->deletedItems.cbegin();)
    {
        paths.removeAt((--it).key());
        // Remove item from it's scene (if it has one).
        if ((*it)->scene())
        {
            (*it)->clearFocus();
            (*it)->scene()->removeItem(*it);
        }
    }
    history.append(step);
    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}

void PathContainer::addHistoryStep(DrawHistoryStep *step)
{
    // Remove all "redo" options.
    truncateHistory();
    // Check that transformed items are actually there. (should not be necessary)
    for (auto it=step->transformedItems.constBegin(); it!=step->transformedItems.constEnd();)
    {
        if (it.key() == nullptr || !paths.contains(it.key()))
        {
            // this should never happen
            warn_msg("Invalid entry in transformed items in history step");
            it = step->transformedItems.erase(it);
        }
        else
            ++it;
    }
    // Check that items with draw tool changes are actually there and valid. (should not be necessary)
    for (auto it=step->drawToolChanges.constBegin(); it!=step->drawToolChanges.constEnd();)
    {
        if (it.key() == nullptr || !paths.contains(it.key()) || (it.key()->type() != BasicGraphicsPath::Type && it.key()->type() != FullGraphicsPath::Type))
        {
            // this should never happen
            warn_msg("Invalid entry in draw tool changes in history step");
            it = step->drawToolChanges.erase(it);
        }
        else
            ++it;
    }
    // Check that items with text property changes are actually there and valid. (should not be necessary)
    for (auto it=step->textPropertiesChanges.constBegin(); it!=step->textPropertiesChanges.constEnd();)
    {
        if (it.key() == nullptr || !paths.contains(it.key()) || it.key()->type() != TextGraphicsItem::Type)
        {
            // this should never happen
            warn_msg("Invalid entry in text property changes in history step");
            it = step->textPropertiesChanges.erase(it);
        }
        else
            ++it;
    }
    history.append(step);

    // Limit history size (if necessary).
    if (history.length() > preferences()->history_length_visible_slides)
        clearHistory(preferences()->history_length_visible_slides);
}


QString color_to_rgba(const QColor &color)
{
    return QLatin1Char('#') + QString::number((color.rgb() << 8) + color.alpha(), 16).rightJustified(8, '0', true);
}

QColor rgba_to_color(const QString &string)
{
    switch (string.length())
    {
    case 9:
        return QColor('#' + string.right(2) + string.mid(1,6));
    default:
        return QColor(string);
    }
}


QDataStream &operator<<(QDataStream &stream, const QGraphicsItem *item)
{
    stream << item->type();
    stream << item->pos();
    stream << item->transform();
    switch (item->type())
    {
    case BasicGraphicsPath::Type:
    {
        debug_msg(DebugDrawing, "write BasicGraphicsPath to stream");
        const BasicGraphicsPath *path = static_cast<const BasicGraphicsPath*>(item);
        stream << quint16(path->_tool.tool());
        stream << path->_tool.pen();
        stream << path->_tool.brush();
        stream << quint16(path->_tool.compositionMode());
        stream << path->coordinates;
        break;
    }
    case FullGraphicsPath::Type:
    {
        debug_msg(DebugDrawing, "write FullGraphicsPath to stream");
        const FullGraphicsPath *path = static_cast<const FullGraphicsPath*>(item);
        stream << quint16(path->_tool.tool());
        stream << path->_tool.pen();
        stream << path->_tool.brush();
        stream << quint16(path->_tool.compositionMode());
        stream << path->coordinates;
        stream << path->pressures;
        break;
    }
    case TextGraphicsItem::Type:
    {
        debug_msg(DebugDrawing, "write TextGraphicsItem to stream");
        const TextGraphicsItem *text = static_cast<const TextGraphicsItem*>(item);
        stream << text->font();
        stream << text->defaultTextColor();
        stream << text->document()->toHtml();
        break;
    }
    }
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QGraphicsItem *&item)
{
    int type;
    stream >> type;
    QPointF pos;
    stream >> pos;
    QTransform transform;
    stream >> transform;
    switch (type)
    {
    case BasicGraphicsPath::Type:
    {
        debug_msg(DebugDrawing, "read BasicGraphicsPath from stream");
        quint16 base_tool;
        stream >> base_tool;
        QPen pen;
        stream >> pen;
        QBrush brush;
        stream >> brush;
        quint16 composition_mode;
        stream >> composition_mode;
        DrawTool tool(Tool::BasicTool(base_tool), 0, pen, brush, QPainter::CompositionMode(composition_mode));
        QVector<QPointF> coordinates;
        stream >> coordinates;
        item = new BasicGraphicsPath(tool, coordinates);
        break;
    }
    case FullGraphicsPath::Type:
    {
        debug_msg(DebugDrawing, "read FullGraphicsPath from stream");
        quint16 base_tool;
        stream >> base_tool;
        QPen pen;
        stream >> pen;
        QBrush brush;
        stream >> brush;
        quint16 composition_mode;
        stream >> composition_mode;
        DrawTool tool(Tool::BasicTool(base_tool), 0, pen, brush, QPainter::CompositionMode(composition_mode));
        QVector<QPointF> coordinates;
        stream >> coordinates;
        QVector<float> pressures;
        stream >> pressures;
        item = new FullGraphicsPath(tool, coordinates, pressures);
        break;
    }
    case TextGraphicsItem::Type:
    {
        debug_msg(DebugDrawing, "read TextGraphicsItem from stream");
        TextGraphicsItem *text = new TextGraphicsItem();
        QFont font;
        stream >> font;
        text->setFont(font);
        QColor color;
        stream >> color;
        text->setDefaultTextColor(color);
        QString html;
        stream >> html;
        text->document()->setHtml(html);
        item = text;
        break;
    }
    default:
        item = nullptr;
        break;
    }
    if (item)
    {
        item->setPos(pos);
        item->setTransform(transform);
    }
    return stream;
}
