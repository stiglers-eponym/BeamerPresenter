// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <cmath>
#include <QGraphicsItem>
#include "src/log.h"
#include "src/drawing/selectiontool.h"

/// Compute length of vector p.
/// TODO: same function is defined in shape recognizer. Combine somewhere.
qreal dist(const QPointF &p) noexcept
{
   return std::sqrt(p.x()*p.x() + p.y()*p.y());
}

SelectionTool::~SelectionTool()
{
   if (_type == SelectPolygon)
       try {
           delete std::get<QPolygonF*>(properties);
       } catch (...) {}
}

void SelectionTool::changeType(const Type newtype)
{
   if (_type == SelectPolygon)
   {
       try {
           delete std::get<QPolygonF*>(properties);
           properties = nullptr;
       } catch (...) {
           properties = nullptr;
       }
   }
    _type = newtype;
}

void SelectionTool::startMove(const QPointF &pos) noexcept
{
    changeType(Move);
    properties = general_properties{pos, pos};
}

void SelectionTool::startRectSelection(const QPointF &pos, const void *scene) noexcept
{
    _scene = scene;
    changeType(SelectRect);
    properties = general_properties{pos, pos};
}

void SelectionTool::startFreehandSelection(const QPointF &pos, const void *scene) noexcept
{
    _scene = scene;
    changeType(SelectPolygon);
    QPolygonF *poly = new QPolygonF();
    *poly << pos;
    properties = poly;
}

void SelectionTool::startRotation(const QPointF &reference, const QPointF &center) noexcept
{
    changeType(Rotate);
    const QPointF vec = reference - center;
    properties = rotation_properties{center, 180/M_PI*std::atan2(vec.y(), vec.x())};
}

void SelectionTool::liveUpdate(const QPointF &pos) noexcept
{
    try {
        switch (_type)
        {
        case SelectRect:
           std::get<general_properties>(properties).current_pos = pos;
            break;
        case SelectPolygon:
        {
            auto &poly = std::get<QPolygonF*>(properties);
            if (!poly)
                poly = new QPolygonF();
            *poly << pos;
            break;
        }
        case Move:
        {
            auto &prop = std::get<general_properties>(properties);
            prop.current_pos = pos;
            const QPointF distance = pos - prop.start_pos;
            QTransform move;
            move.translate(distance.x(), distance.y());
            for (auto it=initial_transforms.cbegin(); it!=initial_transforms.cend(); ++it)
                it.key()->setTransform(*it * move);
            break;
        }
        case Rotate:
            setLiveRotation(pos);
            break;
        case Resize:
            setLiveScaling(pos);
            break;
        default:
            break;
        }
    } catch (const std::bad_variant_access& ex) {
        qWarning() << "wrong type in SelectionTool::properties:" << ex.what();
    } catch (...) {
        qWarning() << "unhandled exception in SelectionTool::liveUpdate" << properties.index();
    }
}

void SelectionTool::setLiveRotation(const QPointF &pos)
{
    const auto &prop = std::get<rotation_properties>(properties);
    const QPointF vec = pos - prop.rotation_center;
    const qreal angle = 180/M_PI*std::atan2(vec.y(), vec.x()) - prop.start_angle;
    QTransform rotation;
    rotation.rotate(angle);
    QTransform transform;
    QPointF new_origin;
    qreal dx, dy;
    for (auto it=initial_transforms.cbegin(); it!=initial_transforms.cend(); ++it)
    {
        // This is constructed by trial and error. It just works.
        // The explanation in the comments might be wrong.
        transform = *it;
        // Don't mess up translation in original transform:
        // reset translation, then do the rotation, then restore translation.
        dx = transform.dx();
        dy = transform.dy();
        transform *= QTransform().translate(-dx, -dy);
        transform *= rotation;
        transform *= QTransform().translate(dx, dy);
        // Apply rotation around origin of the item.
        it.key()->setTransform(transform);
        // Construct new origin of this item:
        // First construct it in scene coordinates, then map to item coordinates.
        new_origin = it.key()->mapFromScene(prop.rotation_center + (it.key()->scenePos() - prop.rotation_center) * rotation);
        // Translate to new origin.
        transform = QTransform().translate(new_origin.x(), new_origin.y()) * transform;
        it.key()->setTransform(transform);
    }
}

void SelectionTool::setLiveScaling(const QPointF &pos)
{
    const auto &prop = std::get<scale_properties>(properties);
    QTransform scale;
    scale.scale(
                prop.reference.x() == prop.start_handle.x()
                    ? 1
                    : (pos.x() - prop.reference.x())/(prop.start_handle.x() - prop.reference.x()),
                prop.reference.y() == prop.start_handle.y()
                    ? 1
                    : (pos.y() - prop.reference.y())/(prop.start_handle.y() - prop.reference.y())
                );
    QTransform transform;
    QPointF new_origin;
    qreal dx, dy;
    for (auto it=initial_transforms.cbegin(); it!=initial_transforms.cend(); ++it)
    {
        transform = *it;
        // Don't mess up translation in original transform:
        // reset translation, then do the rotation, then restore translation.
        dx = transform.dx();
        dy = transform.dy();
        transform *= QTransform().translate(-dx, -dy);
        transform *= scale;
        transform *= QTransform().translate(dx, dy);
        // Apply scaling relative to origin of the item.
        it.key()->setTransform(transform);
        // Construct new origin of this item:
        // First construct it in scene coordinates, then map to item coordinates.
        new_origin = it.key()->mapFromScene(prop.reference + (it.key()->scenePos() - prop.reference) * scale);
        // Translate to new origin.
        transform = QTransform().translate(new_origin.x(), new_origin.y()) * transform;
        it.key()->setTransform(transform);
    }
}

void SelectionTool::startScaling(const QPointF &movable, const QPointF &fixed) noexcept
{
    changeType(Resize);
    properties = scale_properties{movable, fixed};
}

void SelectionTool::reset() noexcept
{
    changeType(NoOperation);
    initial_transforms.clear();
}

void SelectionTool::initTransformations(const QList<QGraphicsItem*> &items) noexcept
{
    initial_transforms.clear();
    for (const auto item : items)
        initial_transforms[item] = item->transform();
}

const QPolygonF SelectionTool::polygon() const noexcept
{
    try {
        if (_type == SelectRect)
        {
            const auto &prop = std::get<general_properties>(properties);
            return QPolygonF(QRectF(prop.start_pos, prop.current_pos).normalized());
        }
        if (_type == SelectPolygon)
            if (auto prop = std::get<QPolygonF*>(properties))
                return *prop;
    } catch (...) {}
    return QPolygonF();
}
