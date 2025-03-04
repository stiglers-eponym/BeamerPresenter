// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/pathcontainer.h"

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QMargins>
#include <QStringList>
#include <QTextDocument>
#include <QTransform>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <iterator>

#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/drawing/graphicspictureitem.h"
#include "src/drawing/textgraphicsitem.h"
#include "src/log.h"
#include "src/names.h"
#include "src/preferences.h"

PathContainer::~PathContainer()
{
  truncateHistory();
  clearHistory();
  for (const auto &[item, count] : _ref_count) delete item;
  _ref_count.clear();
}

void PathContainer::removeFromZOrder(QGraphicsItem *item) noexcept
{
  std::multiset<QGraphicsItem *, decltype(&cmp_by_z)>::const_iterator it =
      _z_order.lower_bound(item);
  while (it != _z_order.cend()) {
    if (*it == item) {
      _z_order.erase(it);
      return;
    } else if ((*it)->zValue() > item->zValue())
      break;
    ++it;
  }
  // Make sure we don't get segfaults because of deleted items in _z_order
  qCritical() << tr("Could not find item in _z_order! Searching full array.");
  it = _z_order.cbegin();
  const auto end = _z_order.cend();
  while (it != end) {
    if (*it == item) {
      _z_order.erase(it);
      return;
    }
    ++it;
  }
  qInfo() << "...did not find item in full array _z_order";
}

void PathContainer::releaseItem(QGraphicsItem *item) noexcept
{
  if (!item) return;
  auto &prop = _ref_count[item];
  --prop.ref_count;
  if (prop.ref_count == 0) {
    if (!prop.visible) {
      debug_msg(DebugDrawing, "deleting item" << item);
      _ref_count.erase(item);
      removeFromZOrder(item);
      delete item;
    }
  } else if (prop.ref_count < 0) {
    debug_msg(DebugDrawing,
              "deleting item, ref_count =" << prop.ref_count << item);
    _ref_count.erase(item);
    removeFromZOrder(item);
    delete item;
  }
}

qreal PathContainer::zValueAfter(const QGraphicsItem *item) const noexcept
{
  if (_z_order.empty())
    return item ? item->zValue() + 10 : 10;
  else if (!item)
    return (*_z_order.crbegin())->zValue() + 10;
  const auto it = _z_order.find(const_cast<QGraphicsItem *>(item));
  if (it == _z_order.cend() || _z_order.size() == 1)
    return std::max(item->zValue(), (*_z_order.crbegin())->zValue()) + 10;
  else
    return ((*std::next(it))->zValue() + item->zValue()) / 2;
}

void PathContainer::deleteStep(const drawHistory::Step &step) noexcept
{
  debug_verbose(DebugDrawing, "deleting history step" << inHistory);
  for (const auto &[item, z] : step.z_value_changes) releaseItem(item);
  for (const auto &[item, z] : step.transformedItems) releaseItem(item);
  for (const auto &[item, z] : step.textPropertiesChanges) releaseItem(item);
  for (const auto &[item, z] : step.drawToolChanges) releaseItem(item);
  for (const auto item : step.createdItems) releaseItem(item);
  for (const auto item : step.deletedItems) releaseItem(item);
}

bool PathContainer::undo(QGraphicsScene *scene)
{
  // Check whether a further entry in history exists.
  if (inHistory < 0 || history.length() - inHistory < 1) return false;

  // Mark that we moved back in history.
  inHistory++;

  const drawHistory::Step &step = history[history.length() - inHistory];

  // 1. Undo transformations.
  if (!step.transformedItems.empty())
    for (const auto &[item, trans] : step.transformedItems)
      item->setTransform(trans.inverted(), true);

  // 2. Undo z value changes
  if (!step.z_value_changes.empty())
    for (const auto &[item, pair] : step.z_value_changes) {
      removeFromZOrder(item);
      item->setZValue(pair.old_z);
      _z_order.insert(item);
    }

  // 3. Undo draw tool changes.
  if (!step.drawToolChanges.empty())
    for (const auto &[item, diff] : step.drawToolChanges) {
      DrawTool tool = item->getTool();
      tool.setPen(diff.old_pen);
      tool.setCompositionMode(diff.old_mode);
      tool.brush() = diff.old_brush;
      item->changeTool(tool);
      item->update();
    }

  // 4. Undo text tool changes.
  if (!step.textPropertiesChanges.empty())
    for (const auto &[item, prop] : step.textPropertiesChanges) {
      item->setFont(prop.old_font);
      item->setDefaultTextColor(
          QColor::fromRgba(item->defaultTextColor().rgba() ^ prop.color_diff));
      item->update();
    }

  // 5. Remove newly created items.
  for (const auto item : step.createdItems) {
    _ref_count[item].visible = false;
    if (item->scene()) {
      item->clearFocus();
      item->scene()->removeItem(item);
    }
  }

  // 6. Restore old items.
  for (const auto item : step.deletedItems)
    if (scene) {
      scene->addItem(item);
      item->show();
      _ref_count[item].visible = true;
    }

  return true;
}

bool PathContainer::redo(QGraphicsScene *scene)
{
  // First check whether there is something to redo in history.
  if (inHistory < 1) return false;

  const drawHistory::Step &step = history[history.length() - inHistory];

  // Move forward in history.
  inHistory--;

  // 1. First remove items which were deleted in this step.
  for (const auto item : step.deletedItems) {
    _ref_count[item].visible = false;
    if (item->scene()) {
      item->clearFocus();
      item->scene()->removeItem(item);
    }
  }

  // 2. Restore newly created items.
  for (const auto item : step.createdItems)
    if (scene) {
      scene->addItem(item);
      item->show();
      _ref_count[item].visible = true;
    }

  // 3. Redo draw tool changes.
  if (!step.textPropertiesChanges.empty())
    for (const auto &[item, prop] : step.textPropertiesChanges) {
      item->setFont(prop.new_font);
      item->setDefaultTextColor(
          QColor::fromRgba(item->defaultTextColor().rgba() ^ prop.color_diff));
      item->update();
    }

  // 4. Redo text tool changes.
  if (!step.drawToolChanges.empty())
    for (const auto &[item, diff] : step.drawToolChanges) {
      DrawTool tool = item->getTool();
      tool.setPen(diff.new_pen);
      tool.setCompositionMode(diff.new_mode);
      tool.brush() = diff.new_brush;
      item->changeTool(tool);
      item->update();
    }

  // 5. Redo z value changes
  if (!step.z_value_changes.empty())
    for (const auto &[item, pair] : step.z_value_changes) {
      removeFromZOrder(item);
      item->setZValue(pair.new_z);
      _z_order.insert(item);
    }

  // 6. Redo transformations.
  if (!step.transformedItems.empty())
    for (const auto &[item, trans] : step.transformedItems)
      item->setTransform(trans, true);

  return true;
}

void PathContainer::truncateHistory()
{
  if (inHistory == -1)
    applyMicroStep();
  else if (inHistory == -2)
    inHistory = 0;
  else {
    // This should never happen, but better check:
    if (inHistory > history.size()) {
      qCritical() << tr("Error in drawing history: inHistory > history.size()");
      inHistory = history.size();
    }
    // Clean up all "redo" options:
    // Delete the last <inHistory> history entries.
    while (inHistory > 0) {
      deleteStep(history.takeLast());
      --inHistory;
    }
  }
}

void PathContainer::clearHistory(int n)
{
  if (inHistory == -2) return;
  if (inHistory == -1) applyMicroStep();

  // Negative values of n don't make any sense and are interpreted as 0.
  if (n < 0) n = 0;

  // Delete the first entries in history until
  // history.length() - inHistory <= n .
  for (int i = history.length() - inHistory; i > n; i--)
    // Take the first step from history and remove it.
    deleteStep(history.takeFirst());
}

bool PathContainer::clearPaths()
{
  if (_ref_count.empty()) return false;
  truncateHistory();
  // Create a new history step.
  history.append(drawHistory::Step());
  auto &deletedItems = history.last().deletedItems;

  // Remove all paths from scene and fill history step.
  for (auto &[item, lookup] : _ref_count)
    if (lookup.visible) {
      lookup.visible = false;
      if (item->scene()) {
        ++(lookup.ref_count);
        deletedItems.append(item);
        item->clearFocus();
        item->scene()->removeItem(item);
      }
    }
  if (history.last().deletedItems.empty()) {
    history.pop_back();
    return false;
  }
  limitHistory();
  return true;
}

void PathContainer::appendForeground(QGraphicsItem *item)
{
  if (!item) return;
  item->setZValue(topZValue() + 10);
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
  if (inHistory != -1) {
    qWarning() << "Tried micro step, but inHistory ==" << inHistory;
    return;
  }
  if (history.empty()) {
    // This should never happen.
    qCritical() << tr("Should apply micro step, but history is empty.");
    inHistory = 0;
    return;
  }

  // Iterate over all paths, filter out visible ones, and check whether they
  // intersect with scene_pos.
  // TODO: how inefficient is it to iterate over all paths (including hidden
  // paths)?
  auto &step = history.last();
  for (const auto &[item, lookup] : _ref_count) {
    if (lookup.visible && item->scene() &&
        item->sceneBoundingRect()
            .marginsAdded(QMargins(size, size, size, size))
            .contains(scene_pos)) {
      QGraphicsScene *scene = item->scene();
      switch (item->type()) {
        case AbstractGraphicsPath::Type:
        case FullGraphicsPath::Type:
        case BasicGraphicsPath::Type: {
          auto path = static_cast<AbstractGraphicsPath *>(item);
          // Apply eraser to path. Get a list of paths obtained by splitting
          // path using the eraser.
          const QList<AbstractGraphicsPath *> list =
              path->splitErase(scene_pos, size);
          // The special case list == {nullptr} is used to indicate that
          // the eraser did not touch the path.
          if (!list.empty() && !list.first()) break;
          // Mark in history step that this path is deleted.
          step.deletedItems.append(path);
          keepItem(path, false);
          // Hide the path, remove it from scene (if possible).
          if (scene) scene->removeItem(path);

          if (list.empty()) break;

          // Path has changed or was split into multiple paths by erasing.
          // Replace path by a QGraphicsItemGroup containing all new paths
          // created from this path.

          // Create the QGraphicsItemGroup.
          const auto group = new QGraphicsItemGroup();
          step.createdItems.append(group);
          keepItem(group);
          // Add all paths in list (which were obtained by erasing in path)
          // to group.
          for (const auto item : list) group->addToGroup(item);
          // Replace path by group in scene (if possible).
          if (scene) {
            scene->addItem(group);
            group->setZValue(path->zValue() + 1e-4);
          } else
            path->hide();
          keepItem(group, true);
          break;
        }
        case QGraphicsItemGroup::Type: {
          // Here a path has already been split into a QGraphicsItemGroup by
          // erasing. Apply eraser again to all items of the group. Within the
          // group, stacking order is irrelevant since all items were created
          // from the same path by erasing.
          const auto group = static_cast<QGraphicsItemGroup *>(item);
          const auto children = group->childItems();
          for (const auto child : children)
            // All items in the group should be paths. But we better check
            // again.
            if (child && (child->type() == FullGraphicsPath::Type ||
                          child->type() == BasicGraphicsPath::Type)) {
              // Apply eraser to child.
              const auto list =
                  static_cast<AbstractGraphicsPath *>(child)->splitErase(
                      scene_pos, size);
              // Again, if list.first() == nullptr, we should do nothing
              // because the eraser did not hit the path.
              if (list.empty() || list.first()) {
                for (auto item : list) group->addToGroup(item);
                group->removeFromGroup(child);
                if (scene) {
                  child->clearFocus();
                  scene->removeItem(child);
                }
                // This item is not stored in any history and can be deleted.
                delete child;  // TODO: Check if this breaks something.
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
  if (inHistory != -1) {
    // TODO: this happens for a double right click
    qWarning() << "Should apply micro step, but inHistory ==" << inHistory;
    return false;
  }
  if (history.empty()) {
    // This should never happen.
    qCritical() << tr("Should apply micro step, but history is empty.");
    inHistory = 0;
    return false;
  }

  debug_msg(DebugDrawing, "applying micro steps. Deleted items:"
                              << history.last().deletedItems);
  const bool finalize =
      preferences()->global_flags & Preferences::FinalizeDrawnPaths;
  auto &step = history.last();
  {
    qreal z;
    QGraphicsScene *scene;
    QGraphicsItemGroup *group;
    QList<QGraphicsItem *> newItems;
#if (QT_VERSION_MAJOR >= 6)
    auto it = step.createdItems.cbegin();
    while (it != step.createdItems.cend())
#else
    auto it = step.createdItems.begin();
    while (it != step.createdItems.end())
#endif
    {
      if (!*it)
        it = step.createdItems.erase(it);
      else if ((*it)->type() == QGraphicsItemGroup::Type) {
        group = static_cast<QGraphicsItemGroup *>(*it);
        z = group->zValue();
        scene = group->scene();
        // TODO: check whether the transformation of the group must be taken
        // into account
        const auto children = group->childItems();
        for (const auto child : children) {
          if (finalize && (child->type() == BasicGraphicsPath::Type ||
                           child->type() == FullGraphicsPath::Type))
            static_cast<AbstractGraphicsPath *>(child)->finalize();
          group->removeFromGroup(child);
          child->setZValue(z);
          keepItem(child, true);
          newItems << child;
          _z_order.insert(child);
        }
        it = step.createdItems.erase(it);
        if (scene) scene->removeItem(group);
        releaseItem(group);
      } else
        ++it;
    }
    step.createdItems << newItems;
  }
  inHistory = 0;
  if (step.empty()) {
    history.removeLast();
    return false;
  }
  limitHistory();
  return true;
}

PathContainer *PathContainer::copy() const noexcept
{
  PathContainer *container = new PathContainer(parent());
  container->inHistory = -2;
  for (const auto &[item, lookup] : _ref_count)
    if (lookup.visible) switch (item->type()) {
        case TextGraphicsItem::Type: {
          const auto *olditem = static_cast<TextGraphicsItem *>(item);
          if (!olditem->isEmpty()) {
            TextGraphicsItem *newitem = olditem->clone();
            newitem->setZValue(olditem->zValue());
            container->keepItem(newitem, true);
            container->_z_order.insert(newitem);
            connect(newitem, &TextGraphicsItem::removeMe, container,
                    &PathContainer::removeItem);
            connect(newitem, &TextGraphicsItem::addMe, container,
                    &PathContainer::addTextItem);
          }
          break;
        }
        case FullGraphicsPath::Type:
        case BasicGraphicsPath::Type: {
          auto newitem = static_cast<AbstractGraphicsPath *>(item)->copy();
          newitem->setZValue(item->zValue());
          container->keepItem(newitem, true);
          container->_z_order.insert(newitem);
          break;
        }
        case GraphicsPictureItem::Type: {
          const auto *olditem = static_cast<GraphicsPictureItem *>(item);
          if (!olditem->empty()) {
            auto newitem = olditem->copy();
            newitem->setZValue(item->zValue());
            container->keepItem(newitem, true);
            container->_z_order.insert(newitem);
          }
          break;
        }
      }
  return container;
}

void PathContainer::writeXml(QXmlStreamWriter &writer) const
{
  std::multiset<QGraphicsItem *, decltype(&cmp_by_z)> itemlist{&cmp_by_z};
  for (const auto &[item, lookup] : _ref_count)
    if (lookup.visible) itemlist.insert(item);
  for (const auto item : itemlist) {
    switch (item->type()) {
      case TextGraphicsItem::Type: {
        const auto text = static_cast<TextGraphicsItem *>(item);
        writer.writeStartElement("text");
        writer.writeAttribute("font", QFontInfo(text->font()).family());
        writer.writeAttribute("size",
                              QString::number(text->font().pointSizeF()));
        writer.writeAttribute(
            "color", color_to_rgba(text->defaultTextColor()).toLower());
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
      case BasicGraphicsPath::Type: {
        const auto path = static_cast<AbstractGraphicsPath *>(item);
        const DrawTool &tool = path->getTool();
        writer.writeStartElement("stroke");
        switch (tool.tool()) {
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
          writer.writeAttribute(
              "style", get_pen_style_codes().value(tool.pen().style()).c_str(),
              "solid");
        if (tool.brush().style() != Qt::NoBrush) {
          // Compare brush and stroke color.
          const QColor &fill = tool.brush().color(),
                       &stroke = tool.pen().color();
          if (fill.red() == stroke.red() && fill.green() == stroke.green() &&
              fill.blue() == stroke.blue()) {
            // Write color in format that is compatible with Xournal++:
            // Save only alpha relative to stroke color (as 8 bit int).
            // avoid division by zero by tiny offset
            float alpha = fill.alphaF() / (tool.pen().color().alphaF() + 1e-6);
            writer.writeAttribute(
                "fill",
                alpha >= 1 ? "255" : QString::number((int)(alpha * 255 + 0.5)));
          } else {
            // Write color to "brushcolor" attribute, which will be ignored by
            // Xournal++
            writer.writeAttribute("brushcolor", color_to_rgba(fill).toLower());
          }
          if (tool.brush().style() != Qt::SolidPattern)
            writer.writeAttribute("brushstyle",
                                  get_brush_style_codes()
                                      .value(tool.brush().style(), "unknown")
                                      .c_str());
        }
        if (tool.compositionMode() != QPainter::CompositionMode_SourceOver)
          writer.writeAttribute("composition",
                                get_composition_mode_codes()
                                    .value(tool.compositionMode(), "unknown")
                                    .c_str());
        writer.writeCharacters(path->stringCoordinates());
        writer.writeEndElement();
        break;
      }
    }
  }
}

AbstractGraphicsPath *loadPath(QXmlStreamReader &reader)
{
  const auto attr = reader.attributes();
  Tool::BasicTool basic_tool = get_string_to_tool().value(
      attr.value("tool").toString(), Tool::InvalidTool);
  if (!(basic_tool & Tool::AnyDrawTool)) return nullptr;
  const QString width_str = attr.value("width").toString();
  if (basic_tool == Tool::Pen && !width_str.contains(' '))
    basic_tool = Tool::FixedWidthPen;
  QPen pen(rgba_to_color(attr.value("color").toString()),
           basic_tool == Tool::Pen ? 1. : width_str.toDouble(),
           get_pen_style_codes().key(
               attr.value("style").toString().toStdString(), Qt::SolidLine),
           Qt::RoundCap, Qt::RoundJoin);
  if (pen.widthF() <= 0) pen.setWidthF(1.);
  // "fill" is the Xournal++ way of storing filling colors. However, it only
  // allows one to add transparency to the stroke color.
  int fill_xopp = attr.value("fill").toInt();
  // "brushcolor" is a BeamerPresenter extension of the Xournal++ file format
  Qt::BrushStyle brush_style = get_brush_style_codes().key(
      attr.value("brushstyle").toString().toStdString(), Qt::SolidPattern);
  QColor fill_color = rgba_to_color(attr.value("brushcolor").toString());
  if (!fill_color.isValid()) {
    fill_color = pen.color();
    if (fill_xopp > 0 && fill_xopp < 256)
      fill_color.setAlphaF(fill_xopp * fill_color.alphaF() / 255);
    else
      brush_style = Qt::NoBrush;
  }
  DrawTool tool(
      basic_tool, Tool::AnyNormalDevice, pen, QBrush(fill_color, brush_style),
      basic_tool == Tool::Highlighter ? QPainter::CompositionMode_Darken
                                      : QPainter::CompositionMode_SourceOver);
  if (attr.hasAttribute("composition")) {
    const QPainter::CompositionMode composition =
        get_composition_mode_codes().key(
            attr.value("composition").toString().toStdString(),
            tool.compositionMode());
    tool.setCompositionMode(composition);
  }
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
  item->setDefaultTextColor(
      rgba_to_color(reader.attributes().value("color").toString()));
  QString transform_string = reader.attributes().value("transform").toString();
  if (transform_string.length() >= 19 &&
      transform_string.startsWith("matrix(") &&
      transform_string.endsWith(")")) {
    // TODO: this is inefficient
    transform_string.remove("matrix(");
    transform_string.remove(")");
    const QStringList list =
        transform_string.trimmed().replace(" ", ",").split(",");
    if (list.length() == 6)
      item->setTransform(QTransform(list[0].toDouble(), list[1].toDouble(),
                                    list[2].toDouble(), list[3].toDouble(),
                                    list[4].toDouble(), list[5].toDouble()));
  }
  const QString text = reader.readElementText();
  if (text.isEmpty()) {
    delete item;
    return nullptr;
  }
  item->setPlainText(text);
  return item;
}

void PathContainer::loadDrawings(QXmlStreamReader &reader)
{
  truncateHistory();
  QGraphicsItem *item;
  history.append(drawHistory::Step());
  while (reader.readNextStartElement()) {
    if (reader.name().toUtf8() == "stroke")
      item = loadPath(reader);
    else if (reader.name().toUtf8() == "text")
      item = loadTextItem(reader);
    else {
      reader.skipCurrentElement();
      continue;
    }
    if (item) {
      // This is equivalent to appendForeground(item), but collects all items in
      // a single history step.
      item->setZValue(topZValue() + 10);
      keepItem(item, true);
      _z_order.insert(item);
      history.last().createdItems.append(item);
    }
  }
  if (history.last().empty())
    history.removeLast();
  else
    limitHistory();
}

void PathContainer::loadDrawings(QXmlStreamReader &reader,
                                 PathContainer *center, PathContainer *left,
                                 PathContainer *right, const qreal page_half)
{
  QGraphicsItem *item;
  if (reader.name().toUtf8() != "layer") {
    qWarning() << "loadDrawings got unexpected XML tag" << reader.name()
               << ", expected 'layer'";
    return;
  }
  const PagePart current_layer = get_page_part_names().key(
      reader.attributes().value("pagePart").toString(), UnknownPagePart);
  while (reader.readNextStartElement()) {
    item = nullptr;
    if (reader.name().toUtf8() == "stroke")
      item = loadPath(reader);
    else if (reader.name().toUtf8() == "text")
      item = loadTextItem(reader);
    if (item) {
      switch (current_layer) {
        case FullPage:
          if (center) center->appendForeground(item);
          break;
        case LeftHalf:
          if (left) left->appendForeground(item);
          break;
        case RightHalf:
          if (right) right->appendForeground(item);
          break;
        default:
          if (center) center->appendForeground(item);
          if (item->sceneBoundingRect().center().x() < page_half) {
            if (left) left->appendForeground(item);
          } else if (right)
            right->appendForeground(item);
          break;
      }
    }
    if (!reader.isEndElement()) reader.skipCurrentElement();
  }
}

QRectF PathContainer::boundingBox() const noexcept
{
  QRectF rect;
  for (const auto &[item, lookup] : _ref_count)
    if (lookup.visible) rect = rect.united(item->sceneBoundingRect());
  return rect;
}

void PathContainer::replaceItem(QGraphicsItem *olditem, QGraphicsItem *newitem)
{
  debug_msg(DebugDrawing, "replace item" << olditem << newitem);
  if (olditem == newitem) return;
  truncateHistory();
  if (olditem && !history.empty()) {
    const auto &laststep = history.last();
    if (laststep.createdItems.size() == 1 &&
        laststep.createdItems.first() == olditem &&
        laststep.deletedItems.size() == 0 &&
        laststep.z_value_changes.size() == 0 &&
        laststep.transformedItems.size() == 0 &&
        laststep.drawToolChanges.size() == 0 &&
        laststep.textPropertiesChanges.size() == 0 &&
        olditem->type() == TextGraphicsItem::Type &&
        static_cast<TextGraphicsItem *>(olditem)->isEmpty()) {
      debug_msg(DebugDrawing, "Deleting empty text item" << olditem);
      const auto it = _ref_count.find(olditem);
      if (it != _ref_count.end()) it->second.visible = false;
      if (olditem->scene()) {
        olditem->clearFocus();
        olditem->scene()->removeItem(olditem);
      }
      undo();
      truncateHistory();
      if (newitem == nullptr) return;
      olditem = nullptr;
    }
  }
  history.append(drawHistory::Step());
  auto &step = history.last();
  if (olditem) {
    keepItem(olditem, false);
    step.deletedItems.append(olditem);
    if (olditem->scene()) {
      olditem->clearFocus();
      olditem->scene()->removeItem(olditem);
    }
  }
  if (newitem) {
    auto &rec_count_entry = _ref_count[newitem];
    rec_count_entry.visible = true;
    if (++rec_count_entry.ref_count > 1) {
      auto it = _z_order.find(newitem);
      while (it != _z_order.end() && *it != newitem &&
             (*it)->zValue() <= newitem->zValue())
        ++it;
      if (*it != newitem) {
        // This should not happen!
        // Make sure we don't get segfaults because of deleted items in _z_order
        qCritical() << tr(
            "New item existed before, but could not find it in _z_order! "
            "Searching full array.");
        it = _z_order.begin();
        const auto end = _z_order.end();
        while (it != end) {
          if (*it == newitem) {
            _z_order.erase(it);
            break;
          }
          ++it;
        }
        _z_order.insert(newitem);
      }
    } else
      _z_order.insert(newitem);
    step.createdItems.append(newitem);
  }
  limitHistory();
}

void PathContainer::addItemsForeground(const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  truncateHistory();
  history.append(drawHistory::Step());
  auto &createdItems = history.last().createdItems;
  qreal z = topZValue();
  for (const auto item : items)
    if (item) {
      z += 10;
      item->setZValue(z);
      keepItem(item, true);
      createdItems.append(item);
      _z_order.insert(item);
    }
  limitHistory();
}

void PathContainer::removeItems(const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  truncateHistory();
  history.append(drawHistory::Step());
  auto &deletedItems = history.last().deletedItems;
  for (const auto item : items)
    if (item) {
      deletedItems.append(item);
      keepItem(item, false);
      if (item->scene()) {
        item->clearFocus();
        item->scene()->removeItem(item);
      }
    }
  limitHistory();
}

bool PathContainer::addChanges(
    std::map<QGraphicsItem *, QTransform> *transforms,
    std::map<AbstractGraphicsPath *, drawHistory::DrawToolDifference> *tools,
    std::map<TextGraphicsItem *, drawHistory::TextPropertiesDifference> *texts)
{
  drawHistory::Step step;
  if (transforms)
    for (const auto &[item, trans] : *transforms)
      if (item) {
        keepItem(item);
        step.transformedItems.insert({item, trans});
      }
  if (tools)
    for (const auto &[item, chng] : *tools)
      if (item) {
        keepItem(item);
        step.drawToolChanges.insert({item, chng});
      }
  if (texts)
    for (const auto &[item, text] : *texts)
      if (item) {
        keepItem(item);
        step.textPropertiesChanges.insert({item, text});
      }
  if (step.empty()) return false;
  truncateHistory();
  history.append(step);
  limitHistory();
  return true;
}

bool PathContainer::isCleared() const noexcept
{
  for (auto &[item, lookup] : _ref_count)
    if (lookup.visible) return false;
  return true;
}

bool PathContainer::bringToForeground(
    const QList<QGraphicsItem *> &to_foreground)
{
  if (_z_order.empty() || to_foreground.empty()) return false;
  const qreal z_top = (*_z_order.crbegin())->zValue();
  // first calculate minimum z value in to_foreground
  qreal z = 1000;
  for (const auto *item : to_foreground)
    if (item->zValue() < z) z = item->zValue();
  // then assign z to a z shift required to bring to_foreground to the
  // foreground
  z = z_top - z;
  if (z < 0 || (z == 0 && _z_order.size() > 1 &&
                (*std::next(_z_order.crbegin()))->zValue() < z_top))
    return false;
  // add an offset of 10
  z += 10;
  history.append(drawHistory::Step());
  auto &changes = history.last().z_value_changes;
  for (const auto item : to_foreground)
    if (item) {
      changes.insert({item, {item->zValue(), item->zValue() + z}});
      keepItem(item);
      removeFromZOrder(item);
      item->setZValue(item->zValue() + z);
      _z_order.insert(item);
    }
  limitHistory();
  return true;
}

bool PathContainer::bringToBackground(
    const QList<QGraphicsItem *> &to_background)
{
  if (_z_order.empty() || to_background.empty()) return false;
  const qreal z_bottom = (*_z_order.cbegin())->zValue();
  // first calculate maximum z value in to_background
  qreal z = 0;
  for (const auto *item : to_background)
    if (item->zValue() > z) z = item->zValue();
  debug_msg(DebugDrawing, "trying to bring to background:" << z << z_bottom);
  if (z <= 1e-100) return false;
  // z is now a scaling prefactor for z values
  z = 0.9 * z_bottom / z;
  if (z <= 0) return false;
  history.append(drawHistory::Step());
  auto &changes = history.last().z_value_changes;
  for (const auto item : to_background)
    if (item) {
      changes.insert({item, {item->zValue(), z * item->zValue()}});
      keepItem(item);
      removeFromZOrder(item);
      item->setZValue(z * item->zValue());
      _z_order.insert(item);
    }
  limitHistory();
  return true;
}

QDataStream &operator<<(QDataStream &stream, const QGraphicsItem *item)
{
  stream << item->type();
  stream << item->pos();
  stream << item->transform();
  switch (item->type()) {
    case BasicGraphicsPath::Type: {
      debug_msg(DebugDrawing, "write BasicGraphicsPath to stream");
      const BasicGraphicsPath *path =
          static_cast<const BasicGraphicsPath *>(item);
      stream << quint16(path->_tool.tool());
      stream << path->_tool.pen();
      stream << path->_tool.brush();
      stream << quint16(path->_tool.compositionMode());
      stream << path->coordinates;
      break;
    }
    case FullGraphicsPath::Type: {
      debug_msg(DebugDrawing, "write FullGraphicsPath to stream");
      const FullGraphicsPath *path =
          static_cast<const FullGraphicsPath *>(item);
      stream << quint16(path->_tool.tool());
      stream << path->_tool.pen();
      stream << path->_tool.brush();
      stream << quint16(path->_tool.compositionMode());
      stream << path->coordinates;
      stream << path->pressures;
      break;
    }
    case TextGraphicsItem::Type: {
      debug_msg(DebugDrawing, "write TextGraphicsItem to stream");
      const TextGraphicsItem *text =
          static_cast<const TextGraphicsItem *>(item);
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
  switch (type) {
    case BasicGraphicsPath::Type: {
      debug_msg(DebugDrawing, "read BasicGraphicsPath from stream");
      quint16 base_tool;
      stream >> base_tool;
      QPen pen;
      stream >> pen;
      QBrush brush;
      stream >> brush;
      quint16 composition_mode;
      stream >> composition_mode;
      DrawTool tool(Tool::BasicTool(base_tool), Tool::NoDevice, pen, brush,
                    QPainter::CompositionMode(composition_mode));
      QVector<QPointF> coordinates;
      stream >> coordinates;
      item = new BasicGraphicsPath(tool, coordinates);
      break;
    }
    case FullGraphicsPath::Type: {
      debug_msg(DebugDrawing, "read FullGraphicsPath from stream");
      quint16 base_tool;
      stream >> base_tool;
      QPen pen;
      stream >> pen;
      QBrush brush;
      stream >> brush;
      quint16 composition_mode;
      stream >> composition_mode;
      DrawTool tool(Tool::BasicTool(base_tool), Tool::NoDevice, pen, brush,
                    QPainter::CompositionMode(composition_mode));
      QVector<QPointF> coordinates;
      stream >> coordinates;
      QVector<float> pressures;
      stream >> pressures;
      item = new FullGraphicsPath(tool, coordinates, pressures);
      break;
    }
    case TextGraphicsItem::Type: {
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
  if (item) {
    item->setPos(pos);
    item->setTransform(transform);
  }
  return stream;
}
