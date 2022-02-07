#include "strokerecognizer.h"

void StrokeRecognizer::calc() noexcept
{
    for (const QPointF &p : stroke->coordinates)
    {
        s += 1;
        sx += p.x();
        sy += p.y();
        sxx += p.x() * p.x();
        sxy += p.x() * p.y();
        syy += p.y() * p.y();
    }
}

BasicGraphicsPath *StrokeRecognizer::recognizeLine() const
{
    if (stroke->size() < 3 || s == 0.)
        return NULL;
    const qreal
            n = sy*sy - s*syy + s*sxx - sx*sx,
            bx = sx/s, // x component of center of the line
            by = sy/s, // y component of center of the line
            d = 2*(sx*sy - s*sxy),
            ay = n - std::sqrt(n*n + d*d),
            loss = (d*d*(s*syy-sy*sy) + ay*ay*(s*sxx-sx*sx) + 2*d*ay*(sx*sy-s*sxy))/(s*s*(d*d+ay*ay));
    if (loss > 0.1) // TODO: configure acceptable loss
        return NULL;
    /*
     * TODO: construct a line centered at (bx, by) with slope given by ay/d.
     * Define start and end point of the line using the bounding rect.
     */
    return NULL;
}
