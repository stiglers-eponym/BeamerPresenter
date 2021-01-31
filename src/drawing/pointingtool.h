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
    QPointF _pos;
    float _size;
    QColor _color;

public:
    PointingTool(const BasicTool tool, const float size, const QColor &color, const int device = AnyDevice) noexcept :
        Tool(tool, device), _size(size), _color(color) {}

    PointingTool(const PointingTool &other) noexcept :
        Tool(other._tool, other._device), _pos(other._pos), _size(other._size), _color(other._color) {}

    const QPointF &pos() const noexcept
    {return _pos;}

    void setPos(const QPointF &pos) noexcept
    {_pos = pos;}

    float size() const noexcept
    {return _size;}

    const QColor &color() const noexcept
    {return _color;}
};

#endif // POINTINGTOOL_H
