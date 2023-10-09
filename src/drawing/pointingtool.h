// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QPointF>
#include <QColor>
#include <QBrush>
#include <QList>
#include "src/config.h"
#include "src/drawing/tool.h"

/**
 * @brief Tool for highlighting or pointing on a slide.
 *
 * Pointing tools include pointer, torch and magnifier. They all need the
 * properties size, position and brush (which is in most cases just a color).
 */
class PointingTool : public Tool
{
protected:
    /// Pointing positions in scene (i.e. page) coordinates.
    /// Multiple positions are possible since some input devices can have
    /// multiple points (e.g. multi-touch devices).
    QList<QPointF> _pos;
    /// Color of the tool or more advanced brush (for pointer)
    QBrush _brush;
    /// Pointer to scene at which this tool is currently active. _scene is used
    /// by slide views to determine whether this tool should be drawn.
    const void *_scene {nullptr};
    /// Radius of drawing tool (in points)
    float _size;
    /// Scale for magnification, only used by magnifier, or line width for eraser.
    float _scale = 2.;

public:
    /// Constructor with full initialization.
    /// @param tool basic tool. Must be a tool for pointing.
    /// @param size tool size (radius in points)
    /// @param brush color or more advanced brush for the tool
    /// @param device input device(s) defined by combination of flags
    /// @param scale For magnifier: magnification factor. For eraser: width of drawn circle.
    PointingTool(const BasicTool tool, const float size, const QBrush &brush, const int device = AnyDevice, const float scale = 2.) noexcept :
        Tool(tool, device), _brush(brush), _size(size), _scale(scale) {}

    /// Copy constructor
    /// @param other tool to be copied
    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _brush(other._brush), _size(other._size), _scale(other._scale) {}

    /// Initialize brush to a fancy pointer. Color and size are taken
    /// from the existing settings.
    void initPointerBrush() noexcept;

    /// @return _pos
    const QList<QPointF> &pos() const noexcept
    {return _pos;}

    /// @return _scene
    const void* &scene() noexcept
    {return _scene;}

    /// @return _scene
    const void *const &scene() const noexcept
    {return _scene;}

    /// clear position(s).
    void clearPos() noexcept
    {_pos.clear();}

    /// set single position.
    void setPos(const QPointF &pos) noexcept
    {_pos = {pos};}

    /// set multiple positions.
    void setPos(const QList<QPointF> &pos) noexcept
    {_pos = pos;}

    /// add another position.
    void addPos(const QPointF &pos) noexcept
    {_pos.append(pos);}

    /// @return _size
    float size() const noexcept
    {return _size;}

    /// @return brush color
    QColor color() const noexcept override
    {return _brush.color();}

    /// Set brush color.
    void setColor(const QColor &color) noexcept override;

    /// @return _brush
    const QBrush &brush() const noexcept
    {return _brush;}

    /// @return _scale
    float scale() const noexcept
    {return _scale;}

    /// set scale.
    void setScale(const float scale) noexcept
    {_scale = scale;}

    bool visible() const noexcept override
    {return !_pos.isEmpty() && _scene;}
};

#endif // POINTINGTOOL_H
