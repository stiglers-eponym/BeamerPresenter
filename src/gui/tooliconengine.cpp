// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QFile>
#include "src/gui/tooliconengine.h"
#include "src/drawing/tool.h"
#include "src/drawing/drawtool.h"
#include "src/preferences.h"

ToolIconEngine::ToolIconEngine(const Tool *tool)
{
    if (!tool)
        return;
    QString iconname = string_to_tool.key(tool->tool());
    iconname.replace(' ', '-');
    QColor bg_full_color;
    if (tool->tool() & Tool::AnyDrawTool)
    {
        const DrawTool *drawtool = static_cast<const DrawTool*>(tool);
        if (drawtool->shape() != DrawTool::Freehand)
        {
            if (drawtool->tool() == Tool::FixedWidthPen && drawtool->shape() != DrawTool::Recognize)
                iconname = "pen";
            iconname += "-";
            iconname += string_to_shape.key(drawtool->shape()).c_str();
        }
        if (drawtool->brush().style() != Qt::NoBrush && drawtool->shape() != DrawTool::Arrow && drawtool->shape() != DrawTool::Line)
        {
            iconname += "-filled";
            bg_full_color = drawtool->brush().color();
        }
    }
    filename = preferences()->icon_path + "/tools/" + iconname + ".svg";
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray data = file.readAll();
    if (data.isEmpty())
        return;
    QColor fg_full_color = tool->color();
    if (!fg_full_color.isValid())
        fg_full_color = Qt::black;
    fg_color = fg_full_color.rgba();
    data.replace("#ff0000", fg_full_color.name(QColor::HexRgb).toUtf8());
    data.replace("opacity:0.99", QString("opacity:%1").arg(fg_full_color.alphaF()).toUtf8());
    if (bg_full_color.isValid())
    {
        bg_color = bg_full_color.rgba();
        data.replace("#ff0100", bg_full_color.name(QColor::HexRgb).toUtf8());
        data.replace("fill-opacity:0.98", QString("fill-opacity:%1").arg(bg_full_color.alphaF()).toUtf8());
    }
    else
        bg_color = 0;
    renderer.load(data);
}

ToolIconEngine::ToolIconEngine(const QString &filename, QRgb fg_color, QRgb bg_color) :
    fg_color(fg_color), bg_color(bg_color), filename(filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray data = file.readAll();
    if (data.isEmpty())
        return;
    const QColor fg_full_color(fg_color);
    data.replace("#ff0000", fg_full_color.name(QColor::HexRgb).toUtf8());
    data.replace("opacity:0.99", QString("opacity:%1").arg(fg_full_color.alphaF()).toUtf8());
    if (bg_color != 0)
    {
        const QColor bg_full_color(bg_color);
        data.replace("#ff0100", bg_full_color.name(QColor::HexRgb).toUtf8());
        data.replace("fill-opacity:0.98", QString("fill-opacity:%1").arg(bg_full_color.alphaF()).toUtf8());
    }
    renderer.load(data);
}

QPixmap ToolIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QPixmap pixmap(size);
    pixmap.fill(QColor(0,0,0,0));
    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();
    return pixmap;
}
