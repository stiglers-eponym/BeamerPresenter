#include "src/drawing/pointingtool.h"

void PointingTool::initPointerBrush() noexcept
{
    QRadialGradient grad(.5, .5, .5);
    grad.setCoordinateMode(QGradient::ObjectMode);
    const QColor color = _brush.color();
    grad.setColorAt(.1, color);
    QColor semitransparent = color;
    semitransparent.setAlpha(12);
    grad.setColorAt(.9, semitransparent);
    semitransparent.setAlpha(0);
    grad.setColorAt(1, semitransparent);
    _brush = QBrush(grad);
    _brush.setColor(color);
}
