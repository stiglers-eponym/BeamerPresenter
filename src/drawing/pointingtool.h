// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QBrush>
#include <QColor>
#include <QList>
#include <QPointF>

#include "src/config.h"
#include "src/drawing/tool.h"

class SlideScene;

/**
 * @brief Tool for highlighting or pointing on a slide.
 *
 * Pointing tools include pointer, torch and magnifier. They all need the
 * properties size, position and brush (which is in most cases just a color).
 */
class PointingTool : public Tool
{
  static constexpr qreal pointer_inner_radius = 0.1;
  static constexpr qreal pointer_outer_radius = 0.9;
  static constexpr int pointer_outer_alpha = 12;

 protected:
  /// Pointing positions in scene (i.e. page) coordinates.
  /// Multiple positions are possible since some input devices can have
  /// multiple points (e.g. multi-touch devices).
  QList<QPointF> _pos;
  /// Color of the tool or more advanced brush (for pointer)
  QBrush _brush;
  /// Pointer to scene at which this tool is currently active. _scene is used
  /// by slide views to determine whether this tool should be drawn.
  SlideScene *_scene{nullptr};
  /// Radius of drawing tool (in points)
  float _size;
  /// Scale for magnification, only used by magnifier, or line width for eraser.
  float _scale = 2.;

  /// Schedule redraw of slide views on current positions.
  void invalidatePos();

 public:
  /// Constructor with full initialization.
  /// @param tool basic tool. Must be a tool for pointing.
  /// @param size tool size (radius in points)
  /// @param brush color or more advanced brush for the tool
  /// @param device input device(s) defined by combination of flags
  /// @param scale For magnifier: magnification factor. For eraser: width of
  /// drawn circle.
  PointingTool(const BasicTool tool, const float size, const QBrush &brush,
               const Tool::InputDevices device = AnyDevice,
               const float scale = 2.) noexcept
      : Tool(tool, device), _brush(brush), _size(size), _scale(scale)
  {
  }

  /// Copy constructor
  /// @param other tool to be copied
  PointingTool(const PointingTool &other) noexcept
      : Tool(other._tool, other._device),
        _pos(other._pos),
        _brush(other._brush),
        _size(other._size),
        _scale(other._scale)
  {
  }

  /// Initialize brush to a fancy pointer. Color and size are taken
  /// from the existing settings.
  void initPointerBrush() noexcept;

  /// @return _pos
  const QList<QPointF> &pos() const noexcept { return _pos; }

  /// @return _scene
  // SlideScene *&scene() noexcept { return _scene; }

  /// @return _scene
  const SlideScene *scene() const noexcept { return _scene; }

  /// clear position and switch scene.
  void setScene(SlideScene *scene)
  {
    clearPos();
    _scene = scene;
  }

  /// clear position(s).
  void clearPos()
  {
    invalidatePos();
    _pos.clear();
  }

  /// set single position.
  void setPos(const QPointF &pos)
  {
    invalidatePos();
    _pos = {pos};
    invalidatePos();
  }

  /// set multiple positions.
  void setPos(const QList<QPointF> &pos)
  {
    invalidatePos();
    _pos = pos;
    invalidatePos();
  }

  /// add another position.
  void addPos(const QPointF &pos);

  /// @return _size
  float size() const noexcept { return _size; }

  /// @return brush color
  QColor color() const noexcept override { return _brush.color(); }

  /// Set brush color.
  void setColor(const QColor &color) noexcept override;

  /// @return _brush
  const QBrush &brush() const noexcept { return _brush; }

  /// @return _scale
  float scale() const noexcept { return _scale; }

  /// set scale.
  void setScale(const float scale) noexcept { _scale = scale; }

  bool visible() const noexcept override { return !_pos.isEmpty() && _scene; }
};

#endif  // POINTINGTOOL_H
