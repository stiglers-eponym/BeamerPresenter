#include "src/drawing/strokerecognizer.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/preferences.h"

qreal distance(const QPointF &p) noexcept
{
   return std::sqrt(p.x()*p.x() + p.y()*p.y());
}

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

BasicGraphicsPath *StrokeRecognizer::recognize() const
{
    return recognizeLine();
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
            ay = n - std::sqrt(n*n + d*d), // dx/dy = ay/d
            loss = (d*d*(s*syy-sy*sy) + ay*ay*(s*sxx-sx*sx) + 2*d*ay*(sx*sy-s*sxy))/((d*d+ay*ay) * (s*sxx - sx*sx + s*syy - sy*sy)),
            margin = stroke->_tool.width()/2;
    debug_msg(DebugDrawing, "recognize line:" << bx << by << ay << d << loss)
    if (loss > 0.01) // TODO: configure acceptable loss
        return NULL;

    QPointF p1, p2;
    if (std::abs(d) < std::abs(ay))
    {
        if (std::abs(d) < 0.01*std::abs(ay))
        {
            p1 = {bx, stroke->top - margin};
            p2 = {bx, stroke->bottom + margin};
        }
        else
        {
            p1 = {bx + d/ay*(stroke->top - margin - by), stroke->top - margin};
            p2 = {bx + d/ay*(stroke->bottom + margin - by), stroke->bottom + margin};
        }
    }
    else
    {
        if (std::abs(ay) < 0.01*std::abs(d))
        {
            p1 = {stroke->left + margin, by};
            p2 = {stroke->right - margin, by};
        }
        else
        {
            p1 = {stroke->left + margin, by + ay/d*(stroke->left + margin - bx)};
            p2 = {stroke->right - margin, by + ay/d*(stroke->right - margin - bx)};
        }
    }
    const int segments = distance(p1-p2)/10 + 2;
    const QPointF delta = (p2-p1)/segments;
    QVector<QPointF> coordinates(segments+1);
    for (int i=0; i<=segments; ++i)
        coordinates[i] = p1 + i*delta;
    const QRectF boundingRect(std::min(p1.x(), p2.x()) - margin, std::min(p1.y(), p2.y()) - margin, std::abs(p2.x()-p1.x()) + 2*margin, std::abs(p2.y()-p1.y()) + 2*margin);
    if (stroke->type() == FullGraphicsPath::Type)
    {
        // TODO: adjust pen width
    }
    debug_msg(DebugDrawing, "creating line" << p1 << p2);
    return new BasicGraphicsPath(stroke->_tool, coordinates, boundingRect);
}
