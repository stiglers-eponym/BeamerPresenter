#ifndef TEXTTOOL_H
#define TEXTTOOL_H

#include <QFont>
#include "src/drawing/tool.h"

/**
 * @brief Tool for adding or editing text.
 *
 * @todo better history
 *
 * @todo more flexible text handling (rich text)
 *
 * @todo change font and move text after it was created.
 */
class TextTool : public Tool
{
    /// font for text
    QFont _font;
    /// text color
    QColor _color;

public:
    /// Copy constructor.
    TextTool(const TextTool& other) :
        Tool(TextInputTool, other._device), _font(other._font), _color(other._color) {}

    /// Constructor with complete initialization.
    TextTool(const QFont &font = QFont(), const QColor &color = Qt::black, const int device = 0) noexcept :
        Tool(TextInputTool, device), _font(font), _color(color) {}

    /// get function for _font
    QFont &font() noexcept
    {return _font;}

    /// constant get function for _font
    const QFont &font() const noexcept
    {return _font;}

    /// constant get function for _color
    const QColor &color() const noexcept
    {return _color;}

    /// set function for _color
    void setColor(const QColor &color) noexcept
    {_color = color;}

    /// set function for _font
    void setFont(const QFont &font) noexcept
    {_font = font;}
};

#endif // TEXTTOOL_H
