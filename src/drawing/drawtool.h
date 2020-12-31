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
    QPen _pen;
    float _opacity = 1.;
    QPainter::CompositionMode composition_mode = QPainter::CompositionMode_SourceOver;

public:
    DrawTool(const DrawTool& other) :
        Tool(other._tool), _pen(other._pen), _opacity(other._opacity), composition_mode(other.composition_mode) {}

    DrawTool(const BasicTool tool, const QPen &pen) noexcept : Tool(tool), _pen(pen) {}

    DrawTool(const BasicTool tool, const QColor &color, float width) noexcept :
        Tool(tool), _pen(QBrush(color), width, Qt::SolidLine, Qt::RoundCap) {}

    const QPen &pen() const noexcept
    {return _pen;}

    float width() const noexcept
    {return _pen.widthF();}

    const QColor color() const noexcept
    {return _pen.color();}

    float opacity() const noexcept
    {return _opacity;}

    QPainter::CompositionMode compositionMode() const noexcept
    {return composition_mode;}

    void setPen(const QPen &pen) noexcept
    {_pen = pen;}

    void setOpacity(const float opacity) noexcept
    {_opacity = opacity;}

    void setCompositionMode(const QPainter::CompositionMode mode) noexcept
    {composition_mode = mode;}
};

#endif // DRAWTOOL_H
