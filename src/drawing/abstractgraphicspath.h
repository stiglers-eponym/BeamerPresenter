// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ABSTRACTGRAPHICSPATH_H
#define ABSTRACTGRAPHICSPATH_H

#include <QDataStream>
#include <QGraphicsItem>
#include <QList>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

#include "src/config.h"
#include "src/drawing/drawtool.h"

/**
 * @brief QGraphicsItem representing a path, abstract class
 *
 * Paths consist of a vector of nodes, a bounding rectangle, and a draw tool.
 * The draw tool and the bounding rectangle coordinates are defined in
 * AbstractGraphicsPath, the vector of nodes is defined in inheriting classes.
 *
 * Coordinates are given as positions in the PDF page, measured in points
 * as floating point values. These are the same units as used in SlideScene.
 *
 * Different implementations of AbstractGraphicsPath can be distinguished by
 * their QGraphicsItem::type().
 *
 * @see BasicGraphicsPath
 * @see FullGraphicsPath
 */
class AbstractGraphicsPath : public QGraphicsItem
{
 protected:
  /**
   * @brief Pen for stroking path.
   *
   * In FullGraphicsPath, pen.width is the reference width which gets
   * rescaled by the pressure of an input device.
   */
  DrawTool _tool;

  /// Cached shape
  QPainterPath shape_cache;

  /// Vector of nodes (coordinates).
  QVector<QPointF> coordinates;

  /// Bounding rect
  QRectF bounding_rect;

  friend class BasicGraphicsPath;
  friend class ShapeRecognizer;
  friend QDataStream &operator<<(QDataStream &stream,
                                 const QGraphicsItem *item);
  friend QDataStream &operator>>(QDataStream &stream, QGraphicsItem *&item);

 public:
  /// Constructor: initialize tool.
  /// @param tool tool for stroking this path
  AbstractGraphicsPath(const DrawTool &tool) noexcept : _tool(tool)
  {
    setFlags(QGraphicsItem::ItemIsSelectable);
  }

  /// Constructor: initialize tool and coordinates.
  /// @param tool tool for stroking this path
  /// @param coordinates vector of coordinates
  AbstractGraphicsPath(const DrawTool &tool,
                       const QVector<QPointF> &coordinates) noexcept
      : _tool(tool), coordinates(coordinates)
  {
    setFlags(QGraphicsItem::ItemIsSelectable);
  }

  /// Bounding rectangle of the drawing (including stroke width).
  /// @return bounding rect
  virtual QRectF boundingRect() const noexcept override
  {
    return bounding_rect;
  }

  /// @return number of nodes of the path
  int size() const noexcept { return coordinates.size(); }

  /// Coordinate of the first node in the path.
  /// @return first point coordinate
  const QPointF firstPoint() const noexcept
  {
    return coordinates.isEmpty() ? QPointF() : coordinates.first();
  }

  /// Coordinate of the last node in the path.
  /// @return last point coordinate
  const QPointF lastPoint() const noexcept
  {
    return coordinates.isEmpty() ? QPointF() : coordinates.last();
  }

  /// Transform item coordinates and cache shape.
  void finalize();

  /// Cache shape (only recalculate if no shape is cached).
  void cacheShape() noexcept { shape_cache = shape(); }

  /// Copy this.
  virtual AbstractGraphicsPath *copy() const = 0;

  /**
   * @brief Erase at position pos.
   *
   * Create list of paths obtained when erasing at position *scene_pos*
   * with round eraser of radius *size*. This list is empty if this path
   * is completely erased. Returns nullptr if nothing was erased.
   *
   * @param scene_pos position of eraser (scene coordinates)
   * @param size radius of eraser
   * @return list of paths after erasing (possibly empty) or
   * {nullptr} if nothing was erased.
   */
  virtual QList<AbstractGraphicsPath *> splitErase(const QPointF &scene_pos,
                                                   const qreal size) const = 0;

  /// @return _tool
  const DrawTool &getTool() const noexcept { return _tool; }

  /// Change tool in-place.
  virtual void changeTool(const DrawTool &newtool) noexcept = 0;

  /// Write nodes coordinates to string for saving using scene coordinates.
  /// @return list of coordinates formatted as string
  virtual const QString stringCoordinates() const noexcept;

  /// Write nodes coordinates to string for writing to SVG using
  /// item coordinates.
  /// @return list of coordinates formatted as string
  virtual const QString svgCoordinates() const noexcept;

  /// Write stroke width(s) to string for saving.
  /// @return single width or list of widths formatted as string
  virtual const QString stringWidth() const noexcept = 0;

  /// Shape: for simplicity taken to be the same for full and basic path.
  virtual QPainterPath shape() const override;
};

#endif  // ABSTRACTGRAPHICSPATH_H
