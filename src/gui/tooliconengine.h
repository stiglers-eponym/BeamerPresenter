// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLICONENGINE_H
#define TOOLICONENGINE_H

#include <QIconEngine>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include "src/config.h"

class Tool;

/**
 * @brief QIconEngine for fancy tool icons
 *
 * These icons are constructed from SVG images, but take their color(s)
 * from tool properties. An SVG image is modified in the following way:
 * - the color #ff0000 is replaced with the foreground (pen) color
 * - the color #ff0100 is replaced with the background (filling) color
 * - the opacity 0.99 is replaced with the foreground (pen) opacity
 * - the fill-opacity 0.98 is replaced with the background (filling) opacity
 */
class ToolIconEngine : public QIconEngine
{
    /// Renderer for icon, reads a modified version of the given SVG image.
    QSvgRenderer renderer;
    /// full path to SVG file
    QString filename;
    /// foreground (pen) color, including alpha channel
    QRgb fg_color;
    /// background (filling) color, including alpha channel
    QRgb bg_color;

public:
    /// Create fancy icons for given tool.
    ToolIconEngine(const Tool *tool);

    /// Create fancy icons from filename and colors.
    ToolIconEngine(const QString &filename, QRgb fg_color, QRgb bg_color);

    /// Trivial destructor.
    ~ToolIconEngine() {}

    /// Create a copy of this.
    QIconEngine *clone() const override
    {return new ToolIconEngine(filename, fg_color, bg_color);}

    /// Paint the icon to painter in given rectangle.
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode, QIcon::State state) override
    {renderer.render(painter, rect);}

    /// Return a pixmap of given size showing the icon.
    QPixmap pixmap(const QSize &size, QIcon::Mode, QIcon::State) override;
};

#endif // TOOLICONENGINE_H
