#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QPointF>
#include <QBrush>
#include <QList>
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
    const void *_scene {NULL};
    /// Radius of drawing tool (in points)
    float _size;
    /// Scale for magnification, only used by magnifier, or line width for eraser.
    float _scale = 2.;

public:
    /// Constructor with full initialization.
    PointingTool(const BasicTool tool, const float size, const QBrush &brush, const int device = AnyDevice, const float scale = 2.) noexcept :
        Tool(tool, device), _brush(brush), _size(size), _scale(scale) {}

    /// Copy constructor
    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _brush(other._brush), _size(other._size), _scale(other._scale) {}

    /// Initialize brush to a fancy pointer. Color and size are taken
    /// from the existing settings.
    void initPointerBrush() noexcept;

    /// get function for _pos
    const QList<QPointF> &pos() const noexcept
    {return _pos;}

    /// get function for _scene
    const void* &scene() noexcept
    {return _scene;}

    /// constant get function for _scene
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

    /// get function for _size
    float size() const noexcept
    {return _size;}

    /// get brush color
    const QColor &color() const noexcept
    {return _brush.color();}

    /// get function for _brush
    const QBrush &brush() const noexcept
    {return _brush;}

    /// get function for _scale
    float scale() const noexcept
    {return _scale;}

    /// set scale.
    void setScale(const float scale) noexcept
    {_scale = scale;}
};

#endif // POINTINGTOOL_H
