#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QPointF>
#include <QBrush>
#include <QList>
#include "src/drawing/tool.h"

/**
 * @class PointingTool
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
    /// Radius of drawing tool (in points)
    float _size;
    /// Scale for magnification, only used by magnifier
    float _scale = 2.;
    /// Color of the tool or more advanced brush (for pointer)
    QBrush _brush;

public:
    PointingTool(const BasicTool tool, const float size, const QBrush &brush, const int device = AnyDevice) noexcept :
        Tool(tool, device), _size(size), _brush(brush) {}

    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _size(other._size), _scale(other._scale), _brush(other._brush) {}

    /// Initialize brush to a fancy pointer. Color and size are taken
    /// from the existing settings.
    void initPointerBrush() noexcept;

    const QList<QPointF> &pos() const noexcept
    {return _pos;}

    void clearPos() noexcept
    {_pos.clear();}

    void setPos(const QPointF &pos) noexcept
    {_pos = {pos};}

    void setPos(const QList<QPointF> &pos) noexcept
    {_pos = pos;}

    void addPos(const QPointF &pos) noexcept
    {_pos.append(pos);}

    float size() const noexcept
    {return _size;}

    const QColor &color() const noexcept
    {return _brush.color();}

    const QBrush &brush() const noexcept
    {return _brush;}

    float scale() const noexcept
    {return _scale;}

    void setScale(const float scale) noexcept
    {_scale = scale;}
};

#endif // POINTINGTOOL_H
