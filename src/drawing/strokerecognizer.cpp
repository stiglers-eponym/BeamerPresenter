#include "src/drawing/strokerecognizer.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/preferences.h"

qreal distance(const QPointF &p) noexcept
{
   return std::sqrt(p.x()*p.x() + p.y()*p.y());
}

/*
void StrokeRecognizer::calc1() noexcept
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
*/

void StrokeRecognizer::calc2() noexcept
{
    if (stroke->type() == FullGraphicsPath::Type)
    {
        const FullGraphicsPath *path = static_cast<const FullGraphicsPath*>(stroke);
        QVector<float>::const_iterator pit = path->pressures.cbegin();
        QVector<QPointF>::const_iterator cit = path->coordinates.cbegin();
        for (; cit!=path->coordinates.cend() && pit!=path->pressures.cend(); ++pit, ++cit)
        {
            sxxx += *pit * cit->x() * cit->x() * cit->x();
            sxxy += *pit * cit->x() * cit->x() * cit->y();
            sxyy += *pit * cit->x() * cit->y() * cit->y();
            syyy += *pit * cit->y() * cit->y() * cit->y();
            sxxxx += *pit * cit->x() * cit->x() * cit->x() * cit->x();
            sxxyy += *pit * cit->x() * cit->x() * cit->y() * cit->y();
            syyyy += *pit * cit->y() * cit->y() * cit->y() * cit->y();
        }
    }
    else
    {
        for (const QPointF &p : stroke->coordinates)
        {
            sxxx += p.x() * p.x() * p.x();
            sxxy += p.x() * p.x() * p.y();
            sxyy += p.x() * p.y() * p.y();
            syyy += p.y() * p.y() * p.y();
            sxxxx += p.x() * p.x() * p.x() * p.x();
            sxxyy += p.x() * p.x() * p.y() * p.y();
            syyyy += p.y() * p.y() * p.y() * p.y();
        }
    }
}

BasicGraphicsPath *StrokeRecognizer::recognize()
{
    findLines();
    BasicGraphicsPath *path = recognizeRect();
    if (path)
        return path;
    path = recognizeLine();
    if (path)
        return path;
    calc2();
    path = recognizeEllipse();
    return path;
}

BasicGraphicsPath *StrokeRecognizer::recognizeLine() const
{
    if (stroke->size() < 3 || moments.s == 0.)
        return NULL;
    const Line line = moments.line();
    const qreal margin = stroke->_tool.width();
    debug_msg(DebugDrawing, "recognize line:" << line.bx << line.by << line.angle << line.loss);
    if (line.loss > preferences()->line_sensitivity)
        return NULL;

    const qreal
            n = moments.sy*moments.sy - moments.s*moments.syy + moments.s*moments.sxx - moments.sx*moments.sx,
            ax = 2*(moments.sx*moments.sy - moments.s*moments.sxy),
            ay = n - std::sqrt(n*n + ax*ax);
    QPointF p1, p2;
    if (std::abs(ax) < std::abs(ay))
    {
        if (std::abs(ax) < preferences()->snap_angle*std::abs(ay))
        {
            p1 = {line.bx, stroke->top + margin};
            p2 = {line.bx, stroke->bottom - margin};
        }
        else
        {
            p1 = {line.bx + ax/ay*(stroke->top + margin - line.by), stroke->top + margin};
            p2 = {line.bx + ax/ay*(stroke->bottom - margin - line.by), stroke->bottom - margin};
        }
    }
    else
    {
        if (std::abs(ay) < preferences()->snap_angle*std::abs(ax))
        {
            p1 = {stroke->left + margin, line.by};
            p2 = {stroke->right - margin, line.by};
        }
        else
        {
            p1 = {stroke->left + margin, line.by + ay/ax*(stroke->left + margin - line.bx)};
            p2 = {stroke->right - margin, line.by + ay/ax*(stroke->right - margin - line.bx)};
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
        tool.setWidth(moments.s/stroke->size());
        return new BasicGraphicsPath(tool, coordinates, boundingRect);
    }
    return new BasicGraphicsPath(stroke->_tool, coordinates, boundingRect);
}


void StrokeRecognizer::findLines() noexcept
{
    // 1. Collect line segments.
    qreal oldloss=-1;
    const int step = stroke->size() > 99 ? stroke->size() / 50 : 1;
    const QPointF *p;
    QList<Line> segment_lines;
    QList<Moments> segment_moments;
    Moments newmoments, oldmoments;
    Line line;
    float weight = 1.;
    debug_msg(DebugDrawing, "Start searching lines");
    for (int i=0, start=0; i<stroke->size(); ++i)
    {
        if (stroke->type() == FullGraphicsPath::Type)
            weight = static_cast<const FullGraphicsPath*>(stroke)->pressures[i];
        p = &stroke->coordinates[i];
        newmoments.s   += weight;
        newmoments.sx  += weight * p->x();
        newmoments.sy  += weight * p->y();
        newmoments.sxx += weight * p->x() * p->x();
        newmoments.sxy += weight * p->x() * p->y();
        newmoments.syy += weight * p->y() * p->y();
        if (i > start + 2 && i % step == 0)
        {
            line = newmoments.line(false);
            if (oldloss >= 0 && (line.loss > 0.005 || (oldloss - line.loss) > 8*step/(i-start)*line.loss))
            {
                segment_moments.append(oldmoments);
                moments += newmoments;
                segment_lines.append(oldmoments.line());
                newmoments.reset();
                start = i;
                oldloss = -1;
            }
            else
            {
                oldmoments = newmoments;
                oldloss = line.loss;
            }
        }
    }
    segment_moments.append(newmoments);
    segment_lines.append(newmoments.line());
    moments += newmoments;

    // 2. Filter and combine line segments.
    const qreal total_var = moments.var();
    oldmoments = {0,0,0,0,0,0};
    for (int i=0; i<segment_lines.size(); ++i)
    {
        if (40*segment_lines[i].weight < total_var)
            continue;
        if (!line_segments.isEmpty())
        {
            if (std::abs(line_segments.last().angle - segment_lines[i].angle) < 0.3 || line_segments.last().angle + M_PI - segment_lines[i].angle < 0.3 || segment_lines[i].angle + M_PI - line_segments.last().angle < 0.3)
            {
                oldmoments += segment_moments[i];
                line = oldmoments.line();
                if (line.loss < 0.005)
                {
                    line_segments.last() = line;
                    continue;
                }
            }
        }
        oldmoments = segment_moments[i];
        line_segments.append(segment_lines[i]);
    }

#ifdef QT_DEBUG
    for (const auto &line : line_segments)
        debug_msg(DebugDrawing, "Line segment" << line.bx << line.by << line.angle << line.weight << line.loss);
#endif
}

BasicGraphicsPath *StrokeRecognizer::recognizeRect()
{
    return NULL;
}


BasicGraphicsPath *StrokeRecognizer::recognizeEllipse() const
{
    qreal ax, ay, rx, ry, mx, my, loss, grad_ax, grad_ay, grad_mx, grad_my, anorm, mnorm;
    const QPointF center = stroke->boundingRect().center();
    mx = center.x();
    my = center.y();
    rx = (stroke->right - stroke->left)/2;
    ry = (stroke->bottom - stroke->top)/2;
    debug_msg(DebugDrawing, "try to recognized ellipse" << mx << my << rx << ry);
    ax = 1./(rx*rx);
    ay = 1./(ry*ry);
    // Check stroke is approximately elliptic
    loss = ellipseLossFunc(mx, my, ax, ay) / (moments.s + 10);
    if (loss > 4*preferences()->ellipse_sensitivity)
        return NULL;
    // Optimize fit parameters
    for (int i=0; i<12; ++i)
    {
        grad_ax = ellipseLossGradient_ax(mx, my, ax, ay);
        grad_ay = ellipseLossGradient_ay(mx, my, ax, ay);
        grad_mx = ellipseLossGradient_mx(mx, my, ax, ay);
        grad_my = ellipseLossGradient_my(mx, my, ax, ay);
        debug_verbose(DebugDrawing, grad_mx*(rx+ry)/moments.s << grad_my*(rx+ry)/moments.s << grad_ax*ax/moments.s << grad_ay*ay/moments.s);
        if (std::abs(grad_mx)*(rx+ry) < 1e-3*moments.s && std::abs(grad_my)*(rx+ry) < 1e-3*moments.s && std::abs(grad_ax)*ax < 1e-3*moments.s && std::abs(grad_ay)*ay < 1e-3*moments.s)
            break;
        // TODO: reasonable choice of step size
        mnorm = 0.05/((1+i*i)*std::sqrt(grad_mx*grad_mx + grad_my*grad_my));
        anorm = 0.1/((1+i*i)*std::sqrt(grad_ax*grad_ax + grad_ay*grad_ay));
        mx -= (rx+ry)*mnorm*grad_mx;
        my -= (rx+ry)*mnorm*grad_my;
        ax -= ax*anorm*grad_ax;
        ay -= ay*anorm*grad_ay;
        debug_verbose(DebugDrawing, mx << my << ax << ay << 1./std::sqrt(std::abs(ax)) << 1./std::sqrt(std::abs(ay)) << ellipseLossFunc(mx, my, ax, ay) / moments.s);
    }
    // Check if the stroke points lie on an ellipse
    loss = ellipseLossFunc(mx, my, ax, ay) / (moments.s + 10);
    debug_msg(DebugDrawing, "    found:" << mx << my << 1./std::sqrt(ax) << 1./std::sqrt(ay) << loss);
    if (loss > preferences()->ellipse_sensitivity)
        return NULL;
    rx = 1./std::sqrt(ax);
    ry = 1./std::sqrt(ay);
    if (std::abs(rx - ry) < preferences()->ellipse_to_circle_snapping*(rx+ry))
        rx = ry = (rx+ry)/2;
    // Check if a full ellipse was drawn (and not only a segment)
    if (distance(stroke->lastPoint() - stroke->firstPoint()) > 0.05*(rx+ry))
    {
        const qreal first_angle = std::atan2(stroke->firstPoint().y()-my, stroke->firstPoint().x()-mx);
        // Angle of last point relative to angle of first point
        qreal rel_last_angle = std::atan2(stroke->lastPoint().y()-my, stroke->lastPoint().x()-mx) - first_angle;
        if (rel_last_angle > M_PI)
            rel_last_angle -= 2*M_PI;
        else if (rel_last_angle < -M_PI)
            rel_last_angle += 2*M_PI;
        if (std::abs(rel_last_angle) > 0.1)
        {
            // diff_angle indicates the direction, in which the ellipse was drawn
            qreal diff_angle = std::atan2(stroke->coordinates[stroke->size()/8+2].y()-my, stroke->coordinates[stroke->size()/8+2].x()-mx) - first_angle;
            if (diff_angle > M_PI)
                diff_angle -= 2*M_PI;
            else if (diff_angle < -M_PI)
                diff_angle += 2*M_PI;
            if (rel_last_angle*diff_angle < 0)
                return NULL;
        }
    }
    const int segments = (rx + ry) * 0.67 + 10;
    const qreal
            phasestep = 2*M_PI / segments,
            margin = stroke->_tool.width();
    QVector<QPointF> coordinates(segments+1);
    for (int i=0; i<segments; ++i)
        coordinates[i] = {mx + rx*std::sin(phasestep*i), my + ry*std::cos(phasestep*i)};
    coordinates[segments] = {mx, my + ry};
    const QRectF boundingRect(mx-rx-margin, my-ry-margin, 2*(rx+margin), 2*(ry+margin));
    debug_msg(DebugDrawing, "recognized ellipse" << mx << my << rx << ry << loss);
    if (stroke->type() == FullGraphicsPath::Type)
    {
        DrawTool tool(stroke->_tool);
        tool.setWidth(moments.s/stroke->size());
        return new BasicGraphicsPath(tool, coordinates, boundingRect);
    }
    return new BasicGraphicsPath(stroke->_tool, coordinates, boundingRect);
}
