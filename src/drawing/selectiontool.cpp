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
    start_pos = pos;
    live_pos = pos;
}

void SelectionTool::startRectSelection(const QPointF &pos) noexcept
{
    _type = Select;
    start_pos = pos;
    live_pos = pos;
}

void SelectionTool::startRotation(const QPointF &reference, const QPointF &center) noexcept
{
    _type = Rotate;
    start_pos = reference;
    live_pos = reference;
    rotation_center = center;
}

QPointF SelectionTool::movePosition(const QPointF &new_position) noexcept
{
    const QPointF diff = new_position - live_pos;
    live_pos = new_position;
    return diff;
}

QTransform SelectionTool::transform() const
{
    QTransform transform;
    switch (_type)
    {
    case Move:
        transform.translate(live_pos.x() - start_pos.x(), live_pos.y() - start_pos.y());
        break;
    case Rotate:
        transform.rotate(rotationAngle());
        break;
    case ScaleTopLeft:
    case ScaleTopRight:
    case ScaleBottomLeft:
    case ScaleBottomRight:
        // TODO
        break;
    default:
        break;
    }
    return transform;
}

qreal SelectionTool::rotationAngle() const noexcept
{
    QPointF
            vec1 = start_pos - rotation_center,
            vec2 = live_pos - rotation_center;
    const qreal
            len1 = QPointF::dotProduct(vec1, vec1),
            prod = QPointF::dotProduct(vec1, vec2);
    const char sign = vec1.x() * vec2.y() - vec1.y() * vec2.x() < 0 ? -1 : 1;
    if (prod == 0.)
        return sign*90;
    else
    {
        vec2 *= len1 / prod;
        return sign*180/M_PI*std::atan2(dist(vec2-vec1), dist(vec1));
    }
}

qreal SelectionTool::setLiveRotation(const QPointF &pos) noexcept
{
    QPointF
            vec1 = live_pos - rotation_center,
            vec2 = pos - rotation_center;
    live_pos = pos;
    const qreal
            len1 = QPointF::dotProduct(vec1, vec1),
            prod = QPointF::dotProduct(vec1, vec2);
    const char sign = vec1.x() * vec2.y() - vec1.y() * vec2.x() < 0 ? -1 : 1;
    if (prod == 0.)
        return sign*90;
    else
    {
        vec2 *= len1 / prod;
        return sign*180/M_PI*std::atan2(dist(vec2-vec1), dist(vec1));
    }
}
