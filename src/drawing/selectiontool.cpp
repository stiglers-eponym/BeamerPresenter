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

QTransform SelectionTool::transform() const
{
    QTransform transform;
    switch (_type)
    {
    case Move:
        transform.translate(properties.general.live_pos.x() - properties.general.start_pos.x(), properties.general.live_pos.y() - properties.general.start_pos.y());
        break;
    case Rotate:
        transform.rotate(rotationAngle());
        break;
    case Resize:
        // TODO
        break;
    default:
        break;
    }
    return transform;
}

qreal SelectionTool::setLiveRotation(const QPointF &pos) noexcept
{
    const QPointF vec = pos - properties.rotate.rotation_center;
    properties.rotate.live_angle = 180/M_PI*std::atan2(vec.y(), vec.x());
    return properties.rotate.live_angle - properties.rotate.start_angle;
}

void SelectionTool::startScaling(const QPointF &movable, const QPointF &fixed) noexcept
{
    _type = Resize;
    properties.scale.start_handle = movable;
    properties.scale.live_handle = movable;
    properties.scale.reference = fixed;
}

QPointF SelectionTool::scale() const noexcept
{
    return QPointF(
                    properties.scale.reference.x() == properties.scale.start_handle.x()
                    ? 1
                    : (properties.scale.live_handle.x() - properties.scale.reference.x())/(properties.scale.start_handle.x() - properties.scale.reference.x()),
                    properties.scale.reference.y() == properties.scale.start_handle.y()
                    ? 1
                    : (properties.scale.live_handle.y() - properties.scale.reference.y())/(properties.scale.start_handle.y() - properties.scale.reference.y())
                );
}

QPointF SelectionTool::setLiveScale(const QPointF &pos) noexcept
{
    const QPointF increment(
                    properties.scale.reference.x() == properties.scale.start_handle.x()
                    ? 1
                    : (pos.x() - properties.scale.reference.x())/(properties.scale.live_handle.x() - properties.scale.reference.x()),
                    properties.scale.reference.y() == properties.scale.start_handle.y()
                    ? 1
                    : (pos.y() - properties.scale.reference.y())/(properties.scale.live_handle.y() - properties.scale.reference.y())
                );
    properties.scale.live_handle = pos;
    return increment;
}
