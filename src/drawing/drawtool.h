#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPen>
#include <QPainter>
#include "src/drawing/tool.h"

/// Tool used to draw strokes. Container class for pen, opacity and
/// composition mode.
class DrawTool : public Tool
{
protected:
    /// Pen for stroking the path. In case of FullGraphicsPath,
    /// PointPressure::pressure is set to _pen.widthF()*event.pressure()
    /// while drawing.
    QPen _pen;

    /// Composition mode used to paint the path.
    QPainter::CompositionMode composition_mode = QPainter::CompositionMode_SourceOver;

public:
    DrawTool(const DrawTool& other) :
        Tool(other._tool, other._device), _pen(other._pen), composition_mode(other.composition_mode) {}

    DrawTool(const BasicTool tool, const int device, const QPen &pen, const QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver) noexcept :
        Tool(tool, device), _pen(pen), composition_mode(mode) {}

    virtual bool operator==(const DrawTool &other) const noexcept
    {return _tool==other._tool && _device==other._device && _pen==other._pen && composition_mode==other.composition_mode;}

    const QPen &pen() const noexcept
    {return _pen;}

    float width() const noexcept
    {return _pen.widthF();}

    const QColor color() const noexcept
    {return _pen.color();}

    QPainter::CompositionMode compositionMode() const noexcept
    {return composition_mode;}

    void setPen(const QPen &pen) noexcept
    {_pen = pen;}

    void setCompositionMode(const QPainter::CompositionMode mode) noexcept
    {composition_mode = mode;}
};

#endif // DRAWTOOL_H
