#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QPointF>
#include "src/drawing/tool.h"

/**
 * @brief Tool for highlighting or pointing on a slide.
 */
class PointingTool : public Tool
{
protected:
    /// Pointing position in scene (i.e. page) coordinates.
    QList<QPointF> _pos;
    /// Radius of drawing tool.
    float _size;
    /// Color of the tool.
    QColor _color;
    /// Scale for magnification, only used by magnifier.
    float _scale = 2.;

public:
    PointingTool(const BasicTool tool, const float size, const QColor &color, const int device = AnyDevice) noexcept :
        Tool(tool, device), _size(size), _color(color) {}

    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _size(other._size), _color(other._color), _scale(other._scale) {}

    const QList<QPointF> &pos() const noexcept
    {return _pos;}

    void clearPos() noexcept
    {_pos.clear();}

    void setPos(const QPointF &pos) noexcept
    {_pos = {pos};}

    void addPos(const QPointF &pos) noexcept
    {_pos.append(pos);}

    float size() const noexcept
    {return _size;}

    const QColor &color() const noexcept
    {return _color;}

    float scale() const noexcept
    {return _scale;}

    void setScale(const float scale) noexcept
    {_scale = scale;}
};

#endif // POINTINGTOOL_H
