// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/drawtool.h"

#include <QMap>
#include <QPainter>
#include <string>

const QMap<DrawTool::Shape, std::string> &get_shape_codes() noexcept
{
  static const QMap<DrawTool::Shape, std::string> shape_codes{
      {DrawTool::Freehand, QT_TRANSLATE_NOOP("DrawTool", "freehand")},
      {DrawTool::Rect, QT_TRANSLATE_NOOP("DrawTool", "rectangle")},
      {DrawTool::Ellipse, QT_TRANSLATE_NOOP("DrawTool", "ellipse")},
      {DrawTool::Line, QT_TRANSLATE_NOOP("DrawTool", "line")},
      {DrawTool::Arrow, QT_TRANSLATE_NOOP("DrawTool", "arrow")},
      {DrawTool::Recognize, QT_TRANSLATE_NOOP("DrawTool", "recognize")},
  };
  return shape_codes;
}

const QMap<Qt::PenStyle, std::string> &get_pen_style_codes() noexcept
{
  static const QMap<Qt::PenStyle, std::string> pen_style_codes{
      {Qt::NoPen, "nopen"},         {Qt::SolidLine, "solid"},
      {Qt::DashLine, "dash"},       {Qt::DotLine, "dot"},
      {Qt::DashDotLine, "dashdot"}, {Qt::DashDotDotLine, "dashdotdot"},
  };
  return pen_style_codes;
}

const QMap<QPainter::CompositionMode, std::string> &
get_composition_mode_codes() noexcept
{
  static const QMap<QPainter::CompositionMode, std::string> map{
      {QPainter::CompositionMode_SourceOver, "source over"},
      {QPainter::CompositionMode_Darken, "darken"},
      {QPainter::CompositionMode_Lighten, "lighten"},
      {QPainter::CompositionMode_Difference, "difference"},
      {QPainter::CompositionMode_Plus, "plus"},
      {QPainter::CompositionMode_Multiply, "multiply"},
      {QPainter::CompositionMode_Screen, "screen"},
      {QPainter::CompositionMode_Overlay, "overlay"},
  };
  return map;
}

const QMap<Qt::BrushStyle, std::string> &get_brush_style_codes() noexcept
{
  static const QMap<Qt::BrushStyle, std::string> brush_style_codes{
      {Qt::NoBrush, "nobrush"},
      {Qt::SolidPattern, "solid-pattern"},
      {Qt::Dense1Pattern, "dense-pattern-1"},
      {Qt::Dense2Pattern, "dense-pattern-2"},
      {Qt::Dense3Pattern, "dense-pattern-3"},
      {Qt::Dense4Pattern, "dense-pattern-4"},
      {Qt::Dense5Pattern, "dense-pattern-5"},
      {Qt::Dense6Pattern, "dense-pattern-6"},
      {Qt::Dense7Pattern, "dense-pattern-7"},
      {Qt::HorPattern, "hatched-horizontal"},
      {Qt::VerPattern, "hatched-vertical"},
      {Qt::CrossPattern, "hatched-cross"},
      {Qt::BDiagPattern, "hatched-top-right"},
      {Qt::FDiagPattern, "hatched-top-left"},
      {Qt::DiagCrossPattern, "hatched-diag-cross"},
  };
  return brush_style_codes;
}
