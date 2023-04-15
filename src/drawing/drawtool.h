// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef DRAWTOOL_H
#define DRAWTOOL_H

#include <QPen>
#include <QPainter>
#include <QMetaEnum>
#include "src/config.h"
#include "src/drawing/tool.h"

/**
 * @brief Tool used to draw strokes.
 *
 * Container class for pen, opacity and composition mode.
 */
class DrawTool : public Tool
{
    Q_GADGET
    Q_DECLARE_TR_FUNCTIONS(DrawTool)

public:
    enum Shape {
        Freehand,
        Rect,
        Ellipse,
        Arrow,
        Line,
        Recognize,
    };
    Q_ENUM(Shape)

protected:
    /// Pen for stroking the path. In case of FullGraphicsPath,
    /// PointPressure::pressure is set to _pen.widthF()*event.pressure()
    /// while drawing.
    QPen _pen;

    /// Brush for filling the path.
    QBrush _brush;

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

static const QMap<std::string, DrawTool::Shape> string_to_shape {
    {QT_TRANSLATE_NOOP("DrawTool", "freehand"), DrawTool::Freehand},
    {QT_TRANSLATE_NOOP("DrawTool", "rectangle"), DrawTool::Rect},
    {QT_TRANSLATE_NOOP("DrawTool", "ellipse"), DrawTool::Ellipse},
    {QT_TRANSLATE_NOOP("DrawTool", "line"), DrawTool::Line},
    {QT_TRANSLATE_NOOP("DrawTool", "arrow"), DrawTool::Arrow},
    {QT_TRANSLATE_NOOP("DrawTool", "recognize"), DrawTool::Recognize},
};

static const QMap<std::string, Qt::PenStyle> string_to_pen_style
{
    {QT_TRANSLATE_NOOP("DrawTool", "nopen"), Qt::NoPen},
    {QT_TRANSLATE_NOOP("DrawTool", "solid"), Qt::SolidLine},
    {QT_TRANSLATE_NOOP("DrawTool", "dash"), Qt::DashLine},
    {QT_TRANSLATE_NOOP("DrawTool", "dot"), Qt::DotLine},
    {QT_TRANSLATE_NOOP("DrawTool", "dashdot"), Qt::DashDotLine},
    {QT_TRANSLATE_NOOP("DrawTool", "dashdotdot"), Qt::DashDotDotLine},
};

static const QMap<std::string, Qt::BrushStyle> string_to_brush_style
{
    {QT_TRANSLATE_NOOP("DrawTool", "NoBrush"), Qt::NoBrush},
    {QT_TRANSLATE_NOOP("DrawTool", "SolidPattern"), Qt::SolidPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense1Pattern"), Qt::Dense1Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense2Pattern"), Qt::Dense2Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense3Pattern"), Qt::Dense3Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense4Pattern"), Qt::Dense4Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense5Pattern"), Qt::Dense5Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense6Pattern"), Qt::Dense6Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "Dense7Pattern"), Qt::Dense7Pattern},
    {QT_TRANSLATE_NOOP("DrawTool", "HorPattern"), Qt::HorPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "VerPattern"), Qt::VerPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "CrossPattern"), Qt::CrossPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "BDiagPattern"), Qt::BDiagPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "FDiagPattern"), Qt::FDiagPattern},
    {QT_TRANSLATE_NOOP("DrawTool", "DiagCrossPattern"), Qt::DiagCrossPattern},
};

#endif // DRAWTOOL_H
