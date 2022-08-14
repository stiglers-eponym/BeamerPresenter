// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include <QColor>
#include <QFont>
#include "src/config.h"
#include "src/drawing/tool.h"

/**
 * @brief Tool for adding or editing text.
 *
 * @todo more flexible text handling (rich text)
 */
class TextTool : public Tool
{
    /// font for text
    QFont _font;
    /// text color
    QColor _color;

public:
    /// Copy constructor.
    /// @param other tool to be copied
    TextTool(const TextTool& other) :
        Tool(TextInputTool, other._device), _font(other._font), _color(other._color) {}

    /// Constructor with full initialization.
    /// @param font font
    /// @param color text color
    /// @param device input device(s) defined by combination of flags
    TextTool(const QFont &font = QFont(), const QColor &color = Qt::black, const int device = 0) noexcept :
        Tool(TextInputTool, device), _font(font), _color(color) {}

    /// @return _font
    QFont &font() noexcept
    {return _font;}

    /// @return _font
    const QFont &font() const noexcept
    {return _font;}

    /// @return _color
    QColor color() const noexcept override
    {return _color;}

    /// set function for _color
    void setColor(const QColor &color) noexcept override
    {_color = color;}

    /// set function for _font
    void setFont(const QFont &font) noexcept
    {_font = font;}
};

#endif // TEXTTOOL_H
