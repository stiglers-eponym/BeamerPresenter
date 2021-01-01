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
        Tool(other._tool, other._device), _pen(other._pen), _opacity(other._opacity), composition_mode(other.composition_mode) {}

    DrawTool(const BasicTool tool, const int device, const QPen &pen) noexcept : Tool(tool, device), _pen(pen) {}

    DrawTool(const BasicTool tool, const int device, const QColor &color, float width) noexcept :
        Tool(tool, device), _pen(QBrush(color), width, Qt::SolidLine, Qt::RoundCap) {}

    virtual bool operator==(const DrawTool &other) const noexcept
    {return _tool==other._tool && _device==other._device && _pen==other._pen && _opacity==other._opacity && composition_mode==other.composition_mode;}

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
