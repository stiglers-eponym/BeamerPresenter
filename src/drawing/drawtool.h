// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPen>
#include <QPainter>
#include "src/drawing/tool.h"

/**
 * @brief Tool used to draw strokes.
 *
 * Container class for pen, opacity and composition mode.
 */
class DrawTool : public Tool
{
public:
    enum Shape {
        Freehand,
        Rect,
        Ellipse,
        Arrow,
        Line,
        Recognize,
    };

protected:
    /// Pen for stroking the path. In case of FullGraphicsPath,
    /// PointPressure::pressure is set to _pen.widthF()*event.pressure()
    /// while drawing.
    QPen _pen;

    /// Brush for filling the path.
    QBrush _brush = QBrush();

    /// Composition mode used to stroking and filling the path.
    /// This mainly distinguishes between pen and highlighter.
    QPainter::CompositionMode composition_mode = QPainter::CompositionMode_SourceOver;

    /// Predefined shape used to draw this path
    Shape _shape;

public:
    /// Copy constructor.
    /// @param other tool to be copied
    DrawTool(const DrawTool& other) :
        Tool(other._tool, other._device), _pen(other._pen), _brush(other._brush), composition_mode(other.composition_mode), _shape(other._shape) {}

    /// Constructor with full initialization.
    /// @param tool basic tool. Must be a tool for drawing.
    /// @param device input device(s) defined by combination of flags
    /// @param pen pen used for stroking path
    /// @param brush brush used for filling path
    /// @param mode composition mode for stroking and filling path
    DrawTool(const BasicTool tool, const int device, const QPen &pen, const QBrush &brush = QBrush(), const QPainter::CompositionMode mode = QPainter::CompositionMode_SourceOver, const Shape shape = Freehand) noexcept :
        Tool(tool, device), _pen(pen), _brush(brush), composition_mode(mode), _shape(shape) {}

    /// Comparison by tool, device, pen, brush, and composition mode.
    virtual bool operator==(const DrawTool &other) const noexcept
    {return _tool==other._tool && _device==other._device && _pen==other._pen && _brush==other._brush && composition_mode==other.composition_mode && _shape==other._shape;}

    /// Comparison by tool, device, pen, brush, and composition mode.
    virtual bool operator!=(const DrawTool &other) const noexcept
    {return _tool!=other._tool || _device!=other._device || _pen!=other._pen || _brush!=other._brush || composition_mode!=other.composition_mode || _shape!=other._shape;}

    /// @return _pen
    QPen &rpen() noexcept
    {return _pen;}

    /// @return _pen
    const QPen &pen() const noexcept
    {return _pen;}

    /// @return stroke width in points
    float width() const noexcept
    {return _pen.widthF();}

    /// @return pen color
    QColor color() const noexcept override
    {return _pen.color();}

    /// Set stroke color.
    void setColor(const QColor &color) noexcept override
    {_pen.setColor(color);}

    /// @return _brush
    const QBrush &brush() const noexcept
    {return _brush;}

    /// @return _brush
    QBrush &brush() noexcept
    {return _brush;}

    /// @return _shape
    Shape shape() const noexcept
    {return _shape;}

    void setShape(Shape shape) noexcept
    {_shape = shape;}

    /// @return composition_mode
    QPainter::CompositionMode compositionMode() const noexcept
    {return composition_mode;}

    /// set compotision_mode
    void setCompositionMode(const QPainter::CompositionMode mode) noexcept
    {composition_mode = mode;}

    /// Overwrite pen.
    void setPen(const QPen &pen) noexcept
    {_pen = pen;}

    /// Set stroke width in points.
    void setWidth(const float width) noexcept
    {_pen.setWidthF(width);}
};

static const QMap<QString, DrawTool::Shape> string_to_shape {
    {"freehand", DrawTool::Freehand},
    {"rectangle", DrawTool::Rect},
    {"ellipse", DrawTool::Ellipse},
    {"line", DrawTool::Line},
    {"arrow", DrawTool::Arrow},
    {"recognize", DrawTool::Recognize},
};

#endif // DRAWTOOL_H
