#include <cmath>
#include "src/drawing/selectiontool.h"
#include "src/preferences.h"

/// Compute length of vector p.
/// TODO: same function is defined in shape recognizer. Combine somewhere.
qreal dist(const QPointF &p) noexcept
{
   return std::sqrt(p.x()*p.x() + p.y()*p.y());
}

void SelectionTool::startMove(const QPointF &pos) noexcept
{
    _type = Move;
    properties.general.start_pos = pos;
    properties.general.live_pos = pos;
}

void SelectionTool::startRectSelection(const QPointF &pos) noexcept
{
    _type = Select;
    properties.general.start_pos = pos;
    properties.general.live_pos = pos;
}

void SelectionTool::startRotation(const QPointF &reference, const QPointF &center) noexcept
{
    _type = Rotate;
    const QPointF vec = reference - center;
    properties.rotate.start_angle = 180/M_PI*std::atan2(vec.y(), vec.x());
    properties.rotate.live_angle = properties.rotate.start_angle;
    properties.rotate.rotation_center = center;
}

QPointF SelectionTool::movePosition(const QPointF &new_position) noexcept
{
    const QPointF diff = new_position - properties.general.live_pos;
    properties.general.live_pos = new_position;
    return diff;
}

void SelectionTool::setLiveMoving(const QPointF &pos) noexcept
{
    properties.general.live_pos = pos;
    QPointF distance = pos - properties.general.start_pos;
    QTransform move;
    move.translate(distance.x(), distance.y());
    QPointF diff;
    for (auto it=initial_transforms.cbegin(); it!=initial_transforms.cend(); ++it)
        it.key()->setTransform(*it * move);
}

void SelectionTool::setLiveRotation(const QPointF &pos) noexcept
{
    const QPointF vec = pos - properties.rotate.rotation_center;
    properties.rotate.live_angle = 180/M_PI*std::atan2(vec.y(), vec.x());
    const qreal angle = properties.rotate.live_angle - properties.rotate.start_angle;
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
        new_origin = it.key()->mapFromScene(properties.rotate.rotation_center + (it.key()->scenePos() - properties.rotate.rotation_center) * rotation);
        // Translate to new origin.
        transform = QTransform().translate(new_origin.x(), new_origin.y()) * transform;
        it.key()->setTransform(transform);
    }
}

void SelectionTool::setLiveScaling(const QPointF &pos) noexcept
{
    properties.scale.live_handle = pos;
    const qreal
            scalex = properties.scale.reference.x() == properties.scale.start_handle.x()
                    ? 1
                    : (pos.x() - properties.scale.reference.x())/(properties.scale.start_handle.x() - properties.scale.reference.x()),
            scaley = properties.scale.reference.y() == properties.scale.start_handle.y()
                    ? 1
                    : (pos.y() - properties.scale.reference.y())/(properties.scale.start_handle.y() - properties.scale.reference.y());
    QTransform scale;
    scale.scale(scalex, scaley);
    QTransform transform;
    QPointF point;
    for (auto it=initial_transforms.cbegin(); it!=initial_transforms.cend(); ++it)
    {
        transform = *it;
        // Here the transformation is reset to avoid the accumulation of numerical errors.
        //it.key()->setTransform(transform);
        //point = it.key()->mapFromScene(properties.scale.reference);
        //transform.translate(point.x(), point.y());
        //transform.scale(scalex, scaley);
        //transform.translate(-point.x(), -point.y());
        transform *= scale;
        it.key()->setTransform(transform);
    }
}

void SelectionTool::startScaling(const QPointF &movable, const QPointF &fixed) noexcept
{
    _type = Resize;
    properties.scale.start_handle = movable;
    properties.scale.live_handle = movable;
    properties.scale.reference = fixed;
}

void SelectionTool::reset() noexcept
{
    _type = NoOperation;
    initial_transforms.clear();
}

void SelectionTool::initTransformations(const QList<QGraphicsItem*> &items) noexcept
{
    initial_transforms.clear();
    for (const auto item : items)
        initial_transforms[item] = item->transform();
}
