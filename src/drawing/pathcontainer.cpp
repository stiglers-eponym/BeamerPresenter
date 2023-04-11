// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QStringList>
#include <QTransform>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextDocument>
#include <QMargins>
#include "src/drawing/pathcontainer.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/selectionrectitem.h"
#include "src/names.h"
#include "src/log.h"
#include "src/preferences.h"


PathContainer::~PathContainer()
{
    truncateHistory();
    clearHistory();
    for (const auto &[item,count] : _ref_count.asKeyValueRange())
        delete item;
    _ref_count.clear();
}

void PathContainer::releaseItem(QGraphicsItem *item) noexcept
{
    if (!item)
        return;
    const int count = --_ref_count[item].ref_count;
    if (count <= 0)
    {
        debug_verbose(DebugDrawing, "deleting item" << item);
        _ref_count.remove(item);
        if (count == 0)
        {
            _z_order.erase(item);
            delete item;
        }
    }
}

qreal PathContainer::zValueAfter(const QGraphicsItem *item) const noexcept
{
    if (_z_order.empty())
        return item ? item->zValue() + 10 : 10;
    else if (!item)
        return (*_z_order.crbegin())->zValue() + 10;
    const auto it = _z_order.find(const_cast<QGraphicsItem*>(item));
    if (it == _z_order.cbegin())
        return (item->zValue() + (*_z_order.cbegin())->zValue())/2;
    else if (it == _z_order.cend())
        return item->zValue() + 10;
    else
        return ((*it)->zValue() + item->zValue())/2;
}

void PathContainer::deleteStep(const drawHistory::Step &step) noexcept
{
    debug_verbose(DebugDrawing, "deleting history step" << inHistory);
    if (!step.z_value_changes.isEmpty())
    {
        auto it = step.z_value_changes.keyBegin();
        const auto end = step.z_value_changes.keyEnd();
        for (; it != end; ++it)
            releaseItem(*it);
    }
    if (!step.transformedItems.isEmpty())
    {
        auto it = step.transformedItems.keyBegin();
        const auto end = step.transformedItems.keyEnd();
        for (; it != end; ++it)
            releaseItem(*it);
    }
    if (!step.textPropertiesChanges.isEmpty())
    {
        auto it = step.textPropertiesChanges.keyBegin();
        const auto end = step.textPropertiesChanges.keyEnd();
        for (; it != end; ++it)
            releaseItem(*it);
    }
    if (!step.drawToolChanges.isEmpty())
    {
        auto it = step.drawToolChanges.keyBegin();
        const auto end = step.drawToolChanges.keyEnd();
        for (; it != end; ++it)
            releaseItem(*it);
    }
    for (const auto item : step.createdItems)
        releaseItem(item);
    for (const auto item : step.deletedItems)
        releaseItem(item);
}

bool PathContainer::undo(QGraphicsScene *scene)
{
    // Check whether a further entry in history exists.
    if (inHistory < 0 || history.length() - inHistory < 1)
        return false;

    // Mark that we moved back in history.
    inHistory++;

    const drawHistory::Step &step = history[history.length() - inHistory];

    // 1. Undo transformations.
    if (!step.transformedItems.isEmpty())
        for (const auto &[item, trans] : step.transformedItems.asKeyValueRange())
            item->setTransform(trans.inverted(), true);

    // 2. Undo z value changes
    if (!step.z_value_changes.isEmpty())
        for (const auto &[item, pair] : step.z_value_changes.asKeyValueRange())
        {
            _z_order.erase(item);
            item->setZValue(pair.old_z);
            _z_order.insert(item);
        }

    // 3. Undo draw tool changes.
    if (!step.drawToolChanges.isEmpty())
        for (const auto &[item, diff] : step.drawToolChanges.asKeyValueRange())
        {
            if (item->type() != FullGraphicsPath::Type && item->type() != BasicGraphicsPath::Type)
            {
                // this should never happen
                warn_msg("History of draw tool changes includes item of invalid type" << item->type());
                continue;
            }
            auto path = static_cast<AbstractGraphicsPath*>(item);
            DrawTool tool = path->getTool();
            tool.setPen(diff.old_pen);
            tool.setCompositionMode(diff.old_mode);
            tool.brush() = diff.old_brush;
            path->changeTool(tool);
            path->update();
        }

    // 4. Undo text tool changes.
    if (!step.textPropertiesChanges.isEmpty())
        for (const auto &[item, prop] : step.textPropertiesChanges.asKeyValueRange())
        {
            if (item->type() != TextGraphicsItem::Type)
            {
                // this should never happen
                warn_msg("History of text propery changes includes item of invalid type" << item->type());
                continue;
            }
            auto text = static_cast<TextGraphicsItem*>(item);
            text->setFont(prop.old_font);
            text->setDefaultTextColor(text->defaultTextColor().rgba() ^ prop.color_diff);
            text->update();
        }

    // 5. Remove newly created items.
    for (const auto item : step.createdItems)
    {
        _ref_count[item].visible = false;
        if (item->scene())
        {
            item->clearFocus();
            item->scene()->removeItem(item);
        }
    }

    // 6. Restore old items.
    for (const auto item : step.deletedItems)
        if (scene)
        {
            scene->addItem(item);
            item->show();
            _ref_count[item].visible = true;
        }

    return true;
}

bool PathContainer::redo(QGraphicsScene *scene)
{
    // First check whether there is something to redo in history.
    if (inHistory < 1)
        return false;

    const drawHistory::Step &step = history[history.length() - inHistory];

    // Move forward in history.
    inHistory--;

    // 1. First remove items which were deleted in this step.
    for (const auto item : step.deletedItems)
    {
        _ref_count[item].visible = false;
        if (item->scene())
        {
            item->clearFocus();
            item->scene()->removeItem(item);
        }
    }

    // 2. Restore newly created items.
    for (const auto item : step.createdItems)
        if (scene)
        {
            scene->addItem(item);
            item->show();
            _ref_count[item].visible = true;
        }

    // 3. Redo draw tool changes.
    if (!step.textPropertiesChanges.isEmpty())
        for (const auto &[item, prop] : step.textPropertiesChanges.asKeyValueRange())
        {
            if (item->type() != TextGraphicsItem::Type)
            {
                // this should never happen
                warn_msg("History of text propery changes includes item of invalid type" << item->type());
                continue;
            }
            auto text = static_cast<TextGraphicsItem*>(item);
            text->setFont(prop.new_font);
            text->setDefaultTextColor(text->defaultTextColor().rgba() ^ prop.color_diff);
            text->update();
        }

    // 4. Redo text tool changes.
    if (!step.drawToolChanges.isEmpty())
        for (const auto &[item, diff] : step.drawToolChanges.asKeyValueRange())
        {
            if (item->type() != FullGraphicsPath::Type && item->type() != BasicGraphicsPath::Type)
            {
                // this should never happen
                warn_msg("History of draw tool changes includes item of invalid type" << item->type());
                continue;
            }
            auto path = static_cast<AbstractGraphicsPath*>(item);
            DrawTool tool = path->getTool();
            tool.setPen(diff.new_pen);
            tool.setCompositionMode(diff.new_mode);
            tool.brush() = diff.new_brush;
            path->changeTool(tool);
            path->update();
        }

    // 5. Redo z value changes
    if (!step.z_value_changes.isEmpty())
        for (const auto &[item, pair] : step.z_value_changes.asKeyValueRange())
        {
            _z_order.erase(item);
            item->setZValue(pair.new_z);
            _z_order.insert(item);
        }

    // 6. Redo transformations.
    if (!step.transformedItems.isEmpty())
        for (const auto &[item, trans] : step.transformedItems.asKeyValueRange())
            item->setTransform(trans, true);

    return true;
}

void PathContainer::truncateHistory()
{
    if (inHistory == -1)
        applyMicroStep();
    else if (inHistory == -2)
        inHistory = 0;
    else
        // Clean up all "redo" options:
        // Delete the last <inHistory> history entries.
        while (inHistory > 0)
        {
            deleteStep(history.takeLast());
            --inHistory;
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
        // Take the first step from history and remove it.
        deleteStep(history.takeFirst());
}

void PathContainer::clearPaths()
{
    truncateHistory();
    // Create a new history step.
    history.append(drawHistory::Step());
    auto &deletedItems = history.last().deletedItems;

    // Remove all paths from scene and fill history step.
    {
        auto it = _ref_count.begin();
        const auto end = _ref_count.end();
        QGraphicsItem *item;
        for (; it!=end; ++it)
            if (it->visible)
            {
                it->visible = false;
                item = it.key();
                if (item->scene())
                {
                    ++(it->ref_count);
                    deletedItems.append(item);
                    item->clearFocus();
                    item->scene()->removeItem(item);
                }
            }
    }
    limitHistory();
}

void PathContainer::appendForeground(QGraphicsItem *item)
{
    if (!item)
        return;
    const qreal z = topZValue();
    item->setZValue(z);
    truncateHistory();
    keepItem(item, true);
    _z_order.insert(item);
    history.append(drawHistory::Step());
    history.last().createdItems.append(item);
    limitHistory();
}

void PathContainer::startMicroStep()
{
    // Remove all "redo" options.
    truncateHistory();
    // Create new, empty history step.
    history.append(drawHistory::Step());
    inHistory = -1;
}

void PathContainer::eraserMicroStep(const QPointF &scene_pos, const qreal size)
{
    if (inHistory != -1 || history.isEmpty())
    {
        qCritical() << "Tried microstep, but inHistory == " << inHistory;
        return;
    }

    // Iterate over all paths, filter out visible ones, and check whether they intersect with scene_pos.
    // TODO: how inefficient is it to iterate over all paths (including hidden paths)?
    auto path_it = _ref_count.begin();
    const auto end = _ref_count.end();
    auto &step = history.last();
    for (; path_it != end; ++path_it)
    {
        if (path_it->visible
            && path_it.key()->scene()
            && path_it.key()->sceneBoundingRect().marginsAdded(QMargins(size, size, size, size)).contains(scene_pos))
        {
            QGraphicsScene *scene = path_it.key()->scene();
            switch (path_it.key()->type())
            {
            case AbstractGraphicsPath::Type:
            case FullGraphicsPath::Type:
            case BasicGraphicsPath::Type:
            {
                auto path = static_cast<AbstractGraphicsPath*>(path_it.key());
                // Apply eraser to path. Get a list of paths obtained by splitting
                // path using the eraser.
                const QList<AbstractGraphicsPath*> list = path->splitErase(scene_pos, size);
                // The special case list == {nullptr} is used to indicate that
                // the eraser did not touch the path.
                if (!list.isEmpty() && !list.first())
                    break;
                // Mark in history step that this path is deleted.
                step.deletedItems.append(path);
                keepItem(path, false);
                // Hide the path, remove it from scene (if possible).
                if (scene)
                    scene->removeItem(path);

                if (list.isEmpty())
                    break;

                // Path has changed or was split into multiple paths by erasing.
                // Replace path by a QGraphicsItemGroup containing all new paths
                // created from this path.

                // Create the QGraphicsItemGroup.
                const auto group = new QGraphicsItemGroup();
                step.createdItems.append(group);
                keepItem(group);
                // Add all paths in list (which were obtained by erasing in path)
                // to group.
                for (const auto item : list)
                    group->addToGroup(item);
                // Replace path by group in scene (if possible).
                if (scene)
                {
                    scene->addItem(group);
                    group->setZValue(path->zValue() + 1e-4);
                }
                else
                    path->hide();
                keepItem(group, true);
                break;
            }
            case QGraphicsItemGroup::Type:
            {
                // Here a path has already been split into a QGraphicsItemGroup by erasing.
                // Apply eraser again to all items of the group.
                // Within the group, stacking order is irrelevant since all items were
                // created from the same path by erasing.
                const auto group = static_cast<QGraphicsItemGroup*>(path_it.key());
                for (const auto child : group->childItems())
                    // All items in the group should be paths. But we better check again.
                    if (child && (child->type() == FullGraphicsPath::Type || child->type() == BasicGraphicsPath::Type))
                    {
                        // Apply eraser to child.
                        const auto list = static_cast<AbstractGraphicsPath*>(child)->splitErase(scene_pos, size);
                        // Again, if list.first() == nullptr, we should do nothing
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
    if (inHistory != -1 || history.isEmpty())
    {
        qCritical() << "Should apply micro step, but inHistory ==" << inHistory;
        inHistory = 0;
        return true;
    }

    debug_msg(DebugDrawing, "applying micro steps. Deleted items:" << history.last().deletedItems);
    const bool finalize = preferences()->global_flags & Preferences::FinalizeDrawnPaths;
    auto &step = history.last();
    {
        qreal z;
        QGraphicsScene *scene;
        QGraphicsItemGroup *group;
        QList<QGraphicsItem*> newItems;
        auto it = step.createdItems.cbegin();
        while (it != step.createdItems.cend())
        {
            if (!*it)
                it = step.createdItems.erase(it);
            else if ((*it)->type() == QGraphicsItemGroup::Type)
            {
                group = static_cast<QGraphicsItemGroup*>(*it);
                z = group->zValue();
                scene = group->scene();
                // TODO: check whether the transformation of the group must be taken into account
                for (const auto child : group->childItems())
                {
                    if (finalize && (child->type() == BasicGraphicsPath::Type || child->type() == FullGraphicsPath::Type))
                        static_cast<AbstractGraphicsPath*>(child)->finalize();
                    group->removeFromGroup(child);
                    child->setZValue(z);
                    keepItem(child, true);
                    newItems << child;
                    _z_order.insert(child);
                }
                it = step.createdItems.erase(it);
                if (scene)
                    scene->removeItem(group);
                releaseItem(group);
            }
            else
                ++it;
        }
        step.createdItems << newItems;
    }
    inHistory = 0;

    limitHistory();
    return !step.deletedItems.isEmpty();
}

PathContainer *PathContainer::copy() const noexcept
{
    PathContainer *container = new PathContainer(parent());
    container->inHistory = -2;
    auto it = _ref_count.keyBegin();
    const auto end = _ref_count.keyEnd();
    for (; it != end; ++it)
        if (*it && (*it)->scene())
            switch ((*it)->type())
            {
            case TextGraphicsItem::Type:
            {
                auto olditem = static_cast<TextGraphicsItem*>(*it);
                if (!olditem->isEmpty())
                {
                    TextGraphicsItem *newitem = olditem->clone();
                    const qreal z = olditem->zValue();
                    newitem->setZValue(z);
                    container->keepItem(newitem, true);
                    container->_z_order.insert(newitem);
                    connect(newitem, &TextGraphicsItem::removeMe, container, &PathContainer::removeItem);
                    connect(newitem, &TextGraphicsItem::addMe, container, &PathContainer::addTextItem);
                }
                break;
            }
            case FullGraphicsPath::Type:
            case BasicGraphicsPath::Type:
            {
                const qreal z = (*it)->zValue();
                auto newitem = static_cast<AbstractGraphicsPath*>(*it)->copy();
                newitem->setZValue(z);
                container->keepItem(newitem, true);
                container->_z_order.insert(newitem);
                break;
            }
            }
    return container;
}

void PathContainer::writeXml(QXmlStreamWriter &writer) const
{
    QList<QGraphicsItem*> itemlist = _ref_count.keys();
    std::sort(itemlist.begin(), itemlist.end(), [](QGraphicsItem *first, QGraphicsItem *second){return first->zValue() > second->zValue();});
    for (const auto item : itemlist)
    {
        switch (item->type())
        {
        case TextGraphicsItem::Type:
        {
            const auto text = static_cast<TextGraphicsItem*>(item);
            writer.writeStartElement("text");
            writer.writeAttribute("font", QFontInfo(text->font()).family());
            writer.writeAttribute("size", QString::number(text->font().pointSizeF()));
            writer.writeAttribute("color", color_to_rgba(text->defaultTextColor()).toLower());
            writer.writeAttribute("x", QString::number(text->x()));
            writer.writeAttribute("y", QString::number(text->y()));
            const QTransform transform = text->transform();
            if (!transform.isIdentity())
                writer.writeAttribute("transform",
                        QString("matrix(%1,%2,%3,%4,%5,%6)")
                            .arg(transform.m11())
                            .arg(transform.m12())
                            .arg(transform.m21())
                            .arg(transform.m22())
                            .arg(transform.dx())
                            .arg(transform.dy()));
            writer.writeCharacters(text->toPlainText());
            writer.writeEndElement();
            break;
        }
        case FullGraphicsPath::Type:
        case BasicGraphicsPath::Type:
        {
            const auto path = static_cast<AbstractGraphicsPath*>(item);
            const DrawTool &tool = path->getTool();
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
            writer.writeAttribute("width", path->stringWidth());
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
            writer.writeCharacters(path->stringCoordinates());
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
    DrawTool tool(
                basic_tool,
                Tool::AnyNormalDevice,
                pen,
                QBrush(fill_color, brush_style),
                basic_tool == Tool::Highlighter ? QPainter::CompositionMode_Darken : QPainter::CompositionMode_SourceOver);
    if (basic_tool == Tool::Pen)
        return new FullGraphicsPath(tool, reader.readElementText(), width_str);
    else
        return new BasicGraphicsPath(tool, reader.readElementText());
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
    item->setDefaultTextColor(rgba_to_color(reader.attributes().value("color").toString()));
    QString transform_string = reader.attributes().value("transform").toString();
    if (transform_string.length() >= 19 && transform_string.startsWith("matrix(") && transform_string.endsWith(")"))
    {
        // TODO: this is inefficient
        transform_string.remove("matrix(");
        transform_string.remove(")");
        const QStringList list = transform_string.trimmed().replace(" ", ",").split(",");
        if (list.length() == 6)
            item->setTransform(QTransform(
                    list[0].toDouble(), list[1].toDouble(),
                    list[2].toDouble(), list[3].toDouble(),
                    list[4].toDouble(), list[5].toDouble()));
    }
    const QString text = reader.readElementText();
    if (text.isEmpty())
    {
        delete item;
        return NULL;
    }
    item->setPlainText(text);
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
            appendForeground(item);
    }
}

void PathContainer::loadDrawings(QXmlStreamReader &reader, PathContainer *left, PathContainer *right, const qreal page_half)
{
    if (!left || !right)
        return;
    while (reader.readNextStartElement())
    {
        if (reader.name().toUtf8() == "stroke")
        {
            const auto path = loadPath(reader);
            if (!path)
                continue;
            if (path->firstPoint().x() > page_half)
                right->appendForeground(path);
            else
                left->appendForeground(path);
        }
        else if (reader.name().toUtf8() == "text")
        {
            const auto item = loadTextItem(reader);
            if (!item)
                continue;
            if (item->pos().x() > page_half)
                right->appendForeground(item);
            else
                left->appendForeground(item);
        }
        else
            reader.skipCurrentElement();
    }
}

QRectF PathContainer::boundingBox() const noexcept
{
    QRectF rect;
    auto it = _ref_count.keyBegin();
    const auto end = _ref_count.keyEnd();
    for (; it != end; ++it)
        if (*it && (*it)->scene())
            rect = rect.united((*it)->sceneBoundingRect());
    return rect;
}

void PathContainer::replaceItem(QGraphicsItem *olditem, QGraphicsItem *newitem)
{
    truncateHistory();
    history.append(drawHistory::Step());
    auto &step = history.last();
    if (olditem)
    {
        keepItem(olditem, false);
        step.deletedItems.append(olditem);
        if (olditem->scene())
        {
            olditem->clearFocus();
            olditem->scene()->removeItem(olditem);
        }
    }
    if (newitem)
    {
        // TODO: better handling of z values
        newitem->setZValue(topZValue() + 10);
        keepItem(newitem, true);
        step.createdItems.append(newitem);
        _z_order.insert(newitem);
    }
    limitHistory();
}

void PathContainer::addItemsForeground(const QList<QGraphicsItem*> &items)
{
    truncateHistory();
    history.append(drawHistory::Step());
    auto &createdItems = history.last().createdItems;
    qreal z = topZValue();
    for (const auto item : items)
        if (item)
        {
            z += 10;
            item->setZValue(z);
            keepItem(item, true);
            createdItems.append(item);
            _z_order.insert(item);
        }
    limitHistory();
}

void PathContainer::removeItems(const QList<QGraphicsItem*> &items)
{
    truncateHistory();
    history.append(drawHistory::Step());
    auto &deletedItems = history.last().deletedItems;
    for (const auto item : items)
        if (item)
        {
            deletedItems.append(item);
            keepItem(item, false);
            if (item->scene())
            {
                item->clearFocus();
                item->scene()->removeItem(item);
            }
        }
    limitHistory();
}

void PathContainer::addChanges(
        QHash<QGraphicsItem*, QTransform> *transforms,
        QHash<QGraphicsItem*, drawHistory::DrawToolDifference> *tools,
        QHash<QGraphicsItem*, drawHistory::TextPropertiesDifference> *texts)
{
    history.append(drawHistory::Step());
    auto &step = history.last();
    if (transforms)
        for (const auto &[item,trans] : transforms->asKeyValueRange())
            if (item)
            {
                step.transformedItems.insert(item, trans);
                keepItem(item);
            }
    if (tools)
        for (const auto &[item,chng] : tools->asKeyValueRange())
            if (item)
            {
                step.drawToolChanges.insert(item, chng);
                keepItem(item);
            }
    if (texts)
        for (const auto &[item,text] : texts->asKeyValueRange())
            if (item)
            {
                step.textPropertiesChanges.insert(item, text);
                keepItem(item);
            }
    if (step.isEmpty())
        history.pop_back();
    else
        limitHistory();
}

bool PathContainer::isCleared() const noexcept
{
    if (_ref_count.isEmpty())
        return true;
    auto it = _ref_count.cbegin();
    const auto &end = _ref_count.cend();
    do {
        if (it->visible) return false;
    } while (++it != end);
    return true;
}

void PathContainer::bringToForeground(const QList<QGraphicsItem*> &to_foreground)
{
    qreal z = 1000;
    for (const auto *item : to_foreground)
        if (item->zValue() < z)
            z = item->zValue();
    z = topZValue() - z;
    if (z <= 0)
        return;
    history.append(drawHistory::Step());
    auto &changes = history.last().z_value_changes;
    for (const auto item : to_foreground)
        if (item)
        {
            z += 10;
            changes.insert(item, {item->zValue(), z});
            keepItem(item);
            _z_order.erase(item);
            item->setZValue(z);
            _z_order.insert(item);
        }
    limitHistory();
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
