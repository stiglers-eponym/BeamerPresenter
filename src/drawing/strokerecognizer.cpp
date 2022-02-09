#include "src/drawing/strokerecognizer.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/preferences.h"

qreal distance(const QPointF &p) noexcept
{
   return std::sqrt(p.x()*p.x() + p.y()*p.y());
}

void StrokeRecognizer::calc() noexcept
{
    if (stroke->type() == FullGraphicsPath::Type)
    {
        const FullGraphicsPath *path = static_cast<const FullGraphicsPath*>(stroke);
        QVector<float>::const_iterator pit = path->pressures.cbegin();
        QVector<QPointF>::const_iterator cit = path->coordinates.cbegin();
        for (; cit!=path->coordinates.cend() && pit!=path->pressures.cend(); ++pit, ++cit)
        {
            s += *pit;
            sx += *pit * cit->x();
            sy += *pit * cit->y();
            sxx += *pit * cit->x() * cit->x();
            sxy += *pit * cit->x() * cit->y();
            syy += *pit * cit->y() * cit->y();
        }
    }
    else
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
}

BasicGraphicsPath *StrokeRecognizer::recognize() const
{
    BasicGraphicsPath *path = recognizeLine();
    if (path)
        return path;
    path = recognizeRect();
    if (path)
        return path;
    path = recognizeEllipse();
    return path;
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
            margin = stroke->_tool.width();
    debug_msg(DebugDrawing, "recognize line:" << bx << by << ay << d << loss)
    if (loss > preferences()->line_sensitivity)
        return NULL;

    QPointF p1, p2;
    if (std::abs(d) < std::abs(ay))
    {
        if (std::abs(d) < preferences()->snap_angle*std::abs(ay))
        {
            p1 = {bx, stroke->top + margin};
            p2 = {bx, stroke->bottom - margin};
        }
        else
        {
            p1 = {bx + d/ay*(stroke->top + margin - by), stroke->top + margin};
            p2 = {bx + d/ay*(stroke->bottom - margin - by), stroke->bottom - margin};
        }
    }
    else
    {
        if (std::abs(ay) < preferences()->snap_angle*std::abs(d))
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
    debug_msg(DebugDrawing, "recognized line" << p1 << p2);
    if (stroke->type() == FullGraphicsPath::Type)
    {
        DrawTool tool(stroke->_tool);
        tool.setWidth(s/stroke->size());
        return new BasicGraphicsPath(tool, coordinates, boundingRect);
    }
    return new BasicGraphicsPath(stroke->_tool, coordinates, boundingRect);
}


BasicGraphicsPath *StrokeRecognizer::recognizeRect() const
{
    return NULL;
}

BasicGraphicsPath *StrokeRecognizer::recognizeEllipse() const
{
    return NULL;
}
