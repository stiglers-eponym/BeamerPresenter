// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/shaperecognizer.h"

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <algorithm>

#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/log.h"
#include "src/preferences.h"

/* Parameters for fitting lines (or rectangles) */
/// minimum number of points in a path used to recognize a line
constexpr int PATH_SEGMENTS_LINE = 50;
/// line loss function threshold
constexpr qreal LINE_LOSS_THRESHOLD = 0.005;
/// threshold for small angles
constexpr qreal ANGLE_THRESHOLD = 0.3;
/// negligible weight of a line segment
constexpr qreal LINE_SEGMENT_NEGLIGIBLE_THRESHOLD = 0.025;

/* Density of points in different shapes */
/// density of points in recognized lines, rectangles, arrows
constexpr int FLAT_POINT_DENSITY = 10;
/// minimum number of points in recognized ellipses
constexpr int CURVED_MIN_POINTS = 10;
/// density of points in ellipse relative to radii (a constant density
/// is added separately)
constexpr qreal CURVED_POINT_DENSITY = 0.67;

/* Parameters in ellipse fit */
/// number of iterations in optimization of ellipse fit parameters
constexpr int ELLIPSE_OPTIMIZE_ITERATIONS = 12;
/// Step in ellipse optimization, coordinates of the center
constexpr qreal ELLIPSE_STEP_CENTER = 0.07;
/// Step in ellipse optimization, radii
constexpr qreal ELLIPSE_STEP_RADII = 0.15;
/// Threshold for fit quality to stop optimizing ellipse parameters
constexpr qreal ELLIPSE_STOP_OPTIMIZING_THRESHOLD = 1e-3;
/// Maximal distance of path start and end point relative to radii in ellipse
constexpr qreal ELLIPSE_START_END_MAX_DISTANCE = 0.1;

/// Compute length of vector p.
inline qreal distance(const QPointF &p) noexcept
{
  return std::sqrt(p.x() * p.x() + p.y() * p.y());
}

/// Compute square of length of vector p.
inline qreal distance_squared(const QPointF &p) noexcept
{
  return p.x() * p.x() + p.y() * p.y();
}

void ShapeRecognizer::calc_higher_moments() noexcept
{
  if (sxxxx > 0) return;
  if (path->type() == FullGraphicsPath::Type) {
    auto fullpath = static_cast<const FullGraphicsPath *>(path);
    auto pit = fullpath->pressures.cbegin();
    const auto pit_end = fullpath->pressures.cend();
    auto cit = fullpath->coordinates.cbegin();
    const auto cit_end = fullpath->coordinates.cend();
    for (; cit != cit_end && pit != pit_end; ++pit, ++cit) {
      sxxx += *pit * cit->x() * cit->x() * cit->x();
      sxxy += *pit * cit->x() * cit->x() * cit->y();
      sxyy += *pit * cit->x() * cit->y() * cit->y();
      syyy += *pit * cit->y() * cit->y() * cit->y();
      sxxxx += *pit * cit->x() * cit->x() * cit->x() * cit->x();
      sxxyy += *pit * cit->x() * cit->x() * cit->y() * cit->y();
      syyyy += *pit * cit->y() * cit->y() * cit->y() * cit->y();
    }
  } else {
    for (const QPointF &p : path->coordinates) {
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

BasicGraphicsPath *ShapeRecognizer::recognize()
{
  findLines();
  BasicGraphicsPath *generated_path = recognizeRect();
  if (generated_path) return generated_path;
  generated_path = recognizeLine();
  if (generated_path) return generated_path;
  calc_higher_moments();
  generated_path = recognizeEllipse();
  return generated_path;
}

BasicGraphicsPath *ShapeRecognizer::recognizeLine() const
{
  if (path->size() < 3 || moments.s == 0.) return nullptr;
  const Line line = moments.line();
  debug_msg(DebugDrawing,
            "recognize line:" << line.bx << line.by << line.angle << line.loss);
  if (line.loss < 0 || line.loss > preferences()->line_sensitivity)
    return nullptr;

  const qreal margin = path->_tool.width() / 2,
              n = moments.sy * moments.sy - moments.s * moments.syy +
                  moments.s * moments.sxx - moments.sx * moments.sx,
              ax = 2 * (moments.sx * moments.sy - moments.s * moments.sxy),
              ay = n - std::sqrt(n * n + ax * ax);
  QPointF p1, p2;
  if (std::abs(ax) < std::abs(ay)) {
    if (std::abs(ax) < preferences()->snap_angle * std::abs(ay)) {
      p1 = {line.bx, path->bounding_rect.top() + margin};
      p2 = {line.bx, path->bounding_rect.bottom() - margin};
    } else {
      p1 = {line.bx + ax / ay * (path->bounding_rect.top() + margin - line.by),
            path->bounding_rect.top() + margin};
      p2 = {
          line.bx + ax / ay * (path->bounding_rect.bottom() - margin - line.by),
          path->bounding_rect.bottom() - margin};
    }
  } else {
    if (std::abs(ay) < preferences()->snap_angle * std::abs(ax)) {
      p1 = {path->bounding_rect.left() + margin, line.by};
      p2 = {path->bounding_rect.right() - margin, line.by};
    } else {
      p1 = {
          path->bounding_rect.left() + margin,
          line.by + ay / ax * (path->bounding_rect.left() + margin - line.bx)};
      p2 = {
          path->bounding_rect.right() - margin,
          line.by + ay / ax * (path->bounding_rect.right() - margin - line.bx)};
    }
  }
  // Create path.
  const QPointF reference = (p1 + p2) / 2, rbegin = (p1 - p2) / 2;
  const int segments = distance(p1 - p2) / 10 + 2;
  const QPointF delta = (p2 - p1) / segments;
  QVector<QPointF> coordinates(segments + 1);
  for (int i = 0; i <= segments; ++i) coordinates[i] = rbegin + i * delta;
  const QRectF boundingRect(std::min(p1.x(), p2.x()) - margin - reference.x(),
                            std::min(p1.y(), p2.y()) - margin - reference.y(),
                            std::abs(p2.x() - p1.x()) + 2 * margin,
                            std::abs(p2.y() - p1.y()) + 2 * margin);
  debug_msg(DebugDrawing, "recognized line" << p1 << p2);
  BasicGraphicsPath *pathitem;
  if (path->type() == FullGraphicsPath::Type) {
    DrawTool tool(path->_tool);
    tool.setWidth(moments.s / path->size());
    pathitem = new BasicGraphicsPath(tool, coordinates, boundingRect);
  } else
    pathitem = new BasicGraphicsPath(path->_tool, coordinates, boundingRect);
  pathitem->setPos(reference + path->pos());
  pathitem->setZValue(path->zValue());
  return pathitem;
}

void ShapeRecognizer::findLines() noexcept
{
  // 1. Collect line segments.
  qreal oldloss = -1;
  const int step = path->size() >= 2 * PATH_SEGMENTS_LINE
                       ? path->size() / PATH_SEGMENTS_LINE
                       : 1;
  const QPointF *p;
  QList<Line> segment_lines;
  QList<Moments> segment_moments;
  Moments newmoments, oldmoments;
  Line line;
  float weight = 1.;
  const auto *pressures =
      path->type() == FullGraphicsPath::Type
          ? &static_cast<const FullGraphicsPath *>(path)->pressures
          : nullptr;
  debug_msg(DebugDrawing, "Start searching lines");
  for (int i = 0, start = 0; i < path->size(); ++i) {
    if (pressures) weight = pressures->at(i);
    p = &path->coordinates[i];
    newmoments.s += weight;
    newmoments.sx += weight * p->x();
    newmoments.sy += weight * p->y();
    newmoments.sxx += weight * p->x() * p->x();
    newmoments.sxy += weight * p->x() * p->y();
    newmoments.syy += weight * p->y() * p->y();
    if (i > start + 2 && i % step == 0) {
      line = newmoments.line(false);
      if (oldloss >= 0 &&
          (line.loss > LINE_LOSS_THRESHOLD ||
           (oldloss - line.loss) > 8 * step / (i - start) * line.loss)) {
        segment_moments.append(oldmoments);
        moments += newmoments;
        segment_lines.append(oldmoments.line());
        newmoments.reset();
        start = i;
        oldloss = -1;
      } else {
        oldmoments = newmoments;
        oldloss = line.loss;
      }
    }
  }
  segment_moments.append(newmoments);
  segment_lines.append(newmoments.line());
  moments += newmoments;

  // 2. Filter and combine line segments.
  const qreal total_var = moments.std();
  oldmoments = {0, 0, 0, 0, 0, 0};
  for (int i = 0; i < segment_lines.size(); ++i) {
    if (segment_lines[i].weight < total_var * LINE_SEGMENT_NEGLIGIBLE_THRESHOLD)
      continue;
    if (!line_segments.isEmpty()) {
      if (std::abs(line_segments.last().angle - segment_lines[i].angle) <
              ANGLE_THRESHOLD ||
          line_segments.last().angle + PI - segment_lines[i].angle <
              ANGLE_THRESHOLD ||
          segment_lines[i].angle + PI - line_segments.last().angle <
              ANGLE_THRESHOLD) {
        oldmoments += segment_moments[i];
        line = oldmoments.line();
        if (line.loss < LINE_LOSS_THRESHOLD) {
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
    debug_msg(DebugDrawing, "Line segment" << line.bx << line.by << line.angle
                                           << line.weight << line.loss);
#endif
}

BasicGraphicsPath *ShapeRecognizer::recognizeRect() const
{
  // Assert that path contains exactly 4 line segments.
  if (line_segments.size() != 4) return nullptr;
  // Check if the path is approximately closed.
  if (distance_squared(path->lastPoint() - path->firstPoint()) >
      preferences()->rect_closing_tolerance * moments.var())
    return nullptr;
  // Compute angle of the rectangle from the 4 segments.
  const qreal total_weight = line_segments[0].weight + line_segments[1].weight +
                             line_segments[2].weight + line_segments[3].weight;
  qreal angle = line_segments[0].angle * line_segments[0].weight +
                line_segments[2].angle * line_segments[2].weight;
  if (std::abs(line_segments[0].angle - line_segments[1].angle - PI / 2) <
      PI / 2)
    angle += (line_segments[1].angle + PI / 2) * line_segments[1].weight;
  else
    angle += (line_segments[1].angle - PI / 2) * line_segments[1].weight;
  if (std::abs(line_segments[0].angle - line_segments[3].angle - PI / 2) <
      PI / 2)
    angle += (line_segments[3].angle + PI / 2) * line_segments[3].weight;
  else
    angle += (line_segments[3].angle - PI / 2) * line_segments[3].weight;
  angle /= total_weight;
  // Check if the angles agree form a rectangle (within some tolerance).
  const qreal angle_tolerance =
      total_weight * preferences()->rect_angle_tolerance;
  if (std::abs(line_segments[0].angle - angle) * line_segments[0].weight >
          angle_tolerance ||
      std::abs(line_segments[2].angle - angle) * line_segments[2].weight >
          angle_tolerance ||
      (std::abs(line_segments[1].angle - PI / 2 - angle) *
               line_segments[1].weight >
           angle_tolerance &&
       std::abs(line_segments[1].angle + PI / 2 - angle) *
               line_segments[1].weight >
           angle_tolerance) ||
      (std::abs(line_segments[3].angle - PI / 2 - angle) *
               line_segments[3].weight >
           angle_tolerance &&
       std::abs(line_segments[3].angle + PI / 2 - angle) *
               line_segments[3].weight >
           angle_tolerance))
    return nullptr;
  // Snap angle to horizontal/vertical orientation.
  if (std::abs(angle) < preferences()->snap_angle ||
      PI - std::abs(angle) < preferences()->snap_angle)
    angle = 0;
  else if (std::abs(angle - PI / 2) < preferences()->snap_angle ||
           std::abs(angle + PI / 2) < preferences()->snap_angle)
    angle = PI / 2;
  // Compute the positions of the corners of the rectangle.
  const qreal ax = std::cos(angle), ay = std::sin(angle);
  qreal alpha = (line_segments[1].bx - line_segments[0].bx) * ax +
                (line_segments[1].by - line_segments[0].by) * ay;
  QPointF p1(line_segments[0].bx + alpha * ax,
             line_segments[0].by + alpha * ay);
  alpha = (line_segments[3].bx - line_segments[0].bx) * ax +
          (line_segments[3].by - line_segments[0].by) * ay;
  QPointF p4(line_segments[0].bx + alpha * ax,
             line_segments[0].by + alpha * ay);
  alpha = (line_segments[1].bx - line_segments[2].bx) * ax +
          (line_segments[1].by - line_segments[2].by) * ay;
  QPointF p2(line_segments[2].bx + alpha * ax,
             line_segments[2].by + alpha * ay);
  alpha = (line_segments[3].bx - line_segments[2].bx) * ax +
          (line_segments[3].by - line_segments[2].by) * ay;
  QPointF p3(line_segments[2].bx + alpha * ax,
             line_segments[2].by + alpha * ay);
  const QPointF reference = (p1 + p2 + p3 + p4) / 4;
  p1 -= reference;
  p2 -= reference;
  p3 -= reference;
  p4 -= reference;

  // Construct the coordinates of points between the corners.
  const qreal dist1 = distance(p2 - p1), dist2 = distance(p3 - p2);
  const int n1 = dist1 / FLAT_POINT_DENSITY + 2,
            n2 = dist2 / FLAT_POINT_DENSITY + 2;
  QVector<QPointF> coordinates(2 * (n1 + n2) + 1);
  const QPointF d12 = (p2 - p1) / n1, d23 = (p3 - p2) / n2,
                d34 = (p4 - p3) / n1, d41 = (p1 - p4) / n2;
  for (int i = 0; i < n1; ++i) coordinates[i] = p1 + i * d12;
  for (int i = 0; i < n2; ++i) coordinates[n1 + i] = p2 + i * d23;
  for (int i = 0; i < n1; ++i) coordinates[n1 + n2 + i] = p3 + i * d34;
  for (int i = 0; i < n2; ++i) coordinates[2 * n1 + n2 + i] = p4 + i * d41;
  coordinates[2 * (n1 + n2)] = p1;

  // Compute the bounding rect.
  const qreal left =
                  std::min(std::min(std::min(p1.x(), p2.x()), p3.x()), p4.x()),
              right =
                  std::max(std::max(std::max(p1.x(), p2.x()), p3.x()), p4.x()),
              top =
                  std::min(std::min(std::min(p1.y(), p2.y()), p3.y()), p4.y()),
              bottom =
                  std::max(std::max(std::max(p1.y(), p2.y()), p3.y()), p4.y()),
              margin = path->_tool.width() / 2;
  const QRectF boundingRect(left - margin, top - margin,
                            right - left + 2 * margin,
                            bottom - top + 2 * margin);

  // Compute the path width (only for FullGraphicsPath).
  BasicGraphicsPath *pathitem;
  if (path->type() == FullGraphicsPath::Type) {
    DrawTool tool(path->_tool);
    tool.setWidth(moments.s / path->size());
    pathitem = new BasicGraphicsPath(tool, coordinates, boundingRect);
  } else
    pathitem = new BasicGraphicsPath(path->_tool, coordinates, boundingRect);
  pathitem->setPos(reference + path->scenePos());
  pathitem->setZValue(path->zValue());
  return pathitem;
}

BasicGraphicsPath *ShapeRecognizer::recognizeEllipse() const
{
  qreal ax, ay, rx, ry, mx, my, loss, grad_ax, grad_ay, grad_mx, grad_my, anorm,
      mnorm;
  const QPointF center = path->boundingRect().center();
  mx = center.x();
  my = center.y();
  rx = (path->bounding_rect.right() - path->bounding_rect.left()) / 2;
  ry = (path->bounding_rect.bottom() - path->bounding_rect.top()) / 2;
  if (rx < 1e-6 || ry < 1e-6) return nullptr;
  debug_msg(DebugDrawing, "try to recognized ellipse" << mx << my << rx << ry);
  ax = 1. / (rx * rx);
  ay = 1. / (ry * ry);
  // Check path is approximately elliptic
  loss = ellipseLossFunc(mx, my, ax, ay) / (moments.s + 10);
  if (loss > 4 * preferences()->ellipse_sensitivity) return nullptr;
  // Optimize fit parameters
  for (int i = 0; i < ELLIPSE_OPTIMIZE_ITERATIONS; ++i) {
    grad_ax = ellipseLossGradient_ax(mx, my, ax, ay);
    grad_ay = ellipseLossGradient_ay(mx, my, ax, ay);
    grad_mx = ellipseLossGradient_mx(mx, my, ax, ay);
    grad_my = ellipseLossGradient_my(mx, my, ax, ay);
    debug_verbose(DebugDrawing, grad_mx * (rx + ry) / moments.s
                                    << grad_my * (rx + ry) / moments.s
                                    << grad_ax * ax / moments.s
                                    << grad_ay * ay / moments.s);
    if (std::abs(grad_mx) * (rx + ry) <
            ELLIPSE_STOP_OPTIMIZING_THRESHOLD * moments.s &&
        std::abs(grad_my) * (rx + ry) <
            ELLIPSE_STOP_OPTIMIZING_THRESHOLD * moments.s &&
        std::abs(grad_ax) * ax <
            ELLIPSE_STOP_OPTIMIZING_THRESHOLD * moments.s &&
        std::abs(grad_ay) * ay < ELLIPSE_STOP_OPTIMIZING_THRESHOLD * moments.s)
      break;
    // TODO: reasonable choice of step size
    mnorm = ELLIPSE_STEP_CENTER /
            ((1 + i * i) * std::sqrt(grad_mx * grad_mx + grad_my * grad_my));
    anorm = ELLIPSE_STEP_RADII /
            ((1 + i * i) * std::sqrt(grad_ax * grad_ax + grad_ay * grad_ay));
    mx -= (rx + ry) * mnorm * grad_mx;
    my -= (rx + ry) * mnorm * grad_my;
    ax -= ax * anorm * grad_ax;
    ay -= ay * anorm * grad_ay;
    debug_verbose(DebugDrawing,
                  mx << my << ax << ay << 1. / std::sqrt(std::abs(ax))
                     << 1. / std::sqrt(std::abs(ay))
                     << ellipseLossFunc(mx, my, ax, ay) / moments.s);
  }
  // Check if the path points lie on an ellipse
  loss = ellipseLossFunc(mx, my, ax, ay) / (moments.s + 10);
  debug_msg(DebugDrawing, "    found:" << mx << my << 1. / std::sqrt(ax)
                                       << 1. / std::sqrt(ay) << loss);
  if (loss > preferences()->ellipse_sensitivity) return nullptr;
  rx = 1. / std::sqrt(ax);
  ry = 1. / std::sqrt(ay);
  if (std::abs(rx - ry) < preferences()->ellipse_to_circle_snapping * (rx + ry))
    rx = ry = (rx + ry) / 2;
  // Check if a full ellipse was drawn (and not only a segment)
  if (distance(path->lastPoint() - path->firstPoint()) >
      ELLIPSE_START_END_MAX_DISTANCE * (rx + ry))
    return nullptr;

  // Construct a path.
  const int segments = (rx + ry) * CURVED_POINT_DENSITY + CURVED_MIN_POINTS;
  const qreal phasestep = 2 * PI / segments, margin = path->_tool.width() / 2;
  QVector<QPointF> coordinates(segments + 1);
  for (int i = 0; i < segments; ++i)
    coordinates[i] = {rx * std::sin(phasestep * i),
                      ry * std::cos(phasestep * i)};
  coordinates[segments] = {0, ry};
  const QRectF boundingRect(-rx - margin, -ry - margin, 2 * (rx + margin),
                            2 * (ry + margin));
  debug_msg(DebugDrawing, "recognized ellipse" << mx << my << rx << ry << loss);
  BasicGraphicsPath *pathitem;
  if (path->type() == FullGraphicsPath::Type) {
    DrawTool tool(path->_tool);
    tool.setWidth(moments.s / path->size());
    pathitem = new BasicGraphicsPath(tool, coordinates, boundingRect);
  } else {
    pathitem = new BasicGraphicsPath(path->_tool, coordinates, boundingRect);
  }
  pathitem->setPos(QPointF(mx, my) + path->pos());
  pathitem->setZValue(path->zValue());
  return pathitem;
}
