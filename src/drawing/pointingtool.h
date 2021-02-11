#ifndef POINTINGTOOL_H
#define POINTINGTOOL_H

#include <QPointF>
#include <QBrush>
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
    QBrush _brush;
    /// Scale for magnification, only used by magnifier.
    float _scale = 2.;

public:
    PointingTool(const BasicTool tool, const float size, const QBrush &brush, const int device = AnyDevice) noexcept :
        Tool(tool, device), _size(size), _brush(brush) {}

    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _size(other._size), _brush(other._brush), _scale(other._scale) {}

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
    {return _brush.color();}

    const QBrush &brush() const noexcept
    {return _brush;}

    float scale() const noexcept
    {return _scale;}

    void setScale(const float scale) noexcept
    {_scale = scale;}
};

#endif // POINTINGTOOL_H
