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
    /// This mainly distinguishes between pen and highlighter.
    QPainter::CompositionMode composition_mode = QPainter::CompositionMode_SourceOver;

public:
    /// Copy constructor.
    DrawTool(const DrawTool& other) :
        Tool(other._tool, other._device), _pen(other._pen), composition_mode(other.composition_mode) {}

    /// Constructor with full initialization.
    DrawTool(const BasicTool tool, const int device, const QPen &pen, const QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver) noexcept :
        Tool(tool, device), _pen(pen), composition_mode(mode) {}

    /// Comparison by tool, device, pen and composition mode.
    virtual bool operator==(const DrawTool &other) const noexcept
    {return _tool==other._tool && _device==other._device && _pen==other._pen && composition_mode==other.composition_mode;}

    /// get function for _pen
    const QPen &pen() const noexcept
    {return _pen;}

    /// Stroke width (pen/highlighter) in points.
    float width() const noexcept
    {return _pen.widthF();}

    /// Pen color.
    const QColor color() const noexcept
    {return _pen.color();}

    /// get function for composition_mode
    QPainter::CompositionMode compositionMode() const noexcept
    {return composition_mode;}

    /// set compotision_mode
    void setCompositionMode(const QPainter::CompositionMode mode) noexcept
    {composition_mode = mode;}

    /// Overwrite pen.
    void setPen(const QPen &pen) noexcept
    {_pen = pen;}

    /// Set width in points.
    void setWidth(const float width) noexcept
    {_pen.setWidthF(width);}
};

#endif // DRAWTOOL_H
