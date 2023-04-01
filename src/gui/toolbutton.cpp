// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QTabletEvent>
#include <QTouchEvent>
#include <QMouseEvent>
#include <QString>
#include <QSize>
#include <QColor>
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include "src/gui/toolbutton.h"
#include "src/preferences.h"
#include "src/drawing/drawtool.h"

ToolButton::ToolButton(Tool *tool, QWidget *parent) noexcept :
        QToolButton(parent),
        tool(nullptr)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(16, 16);
    setIconSize({32,32});
    setContentsMargins(0,0,0,0);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setTool(tool);
}

ToolButton::~ToolButton()
{
    delete tool;
}

void ToolButton::setTool(Tool *newtool)
{
    if (!newtool)
        return;
    QColor color;
    QString iconname = string_to_tool.key(newtool->tool());
    iconname.replace(' ', '-');
    if (newtool->tool() & Tool::AnyDrawTool)
    {
        const DrawTool *drawtool = static_cast<const DrawTool*>(newtool);
        if (drawtool->shape() != DrawTool::Freehand)
        {
            if (drawtool->tool() == Tool::FixedWidthPen && drawtool->shape() != DrawTool::Recognize)
                iconname = "pen";
            iconname += "-";
            iconname += string_to_shape.key(drawtool->shape());
        }
        if (drawtool->brush().style() != Qt::NoBrush && drawtool->shape() != DrawTool::Arrow && drawtool->shape() != DrawTool::Line)
            iconname += "-filled";
        color = drawtool->color();
    }
    else
        color = newtool->color();
    if (!color.isValid())
        color = Qt::black;
    const QString filename = preferences()->icon_path + "/tools/" + iconname + ".svg";
    QSize newsize = size();
    setIconSize(newsize);
    QIcon icon;
    if (color.isValid())
    {
        if (newsize.height() > newsize.width())
            newsize.rheight() = newsize.width();
        else
            newsize.rwidth() = newsize.height();
        const QImage image = fancyIcon(filename, newsize, color);
        if (!image.isNull())
            icon = QIcon(QPixmap::fromImage(image));
    }
    if (icon.isNull())
        icon = QIcon(filename);
    if (icon.isNull())
        setText(string_to_tool.key(newtool->tool()));
    else
        setIcon(icon);
    if (tool != newtool)
    {
        delete tool;
        tool = newtool;
        setToolTip(tr(tool_to_description(tool->tool())));
    }
}

const QImage fancyIcon(const QString &filename, const QSize &size, const QColor &color)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return QImage();
    QByteArray data = file.readAll();
    if (data.isEmpty())
        return QImage();
    data.replace("#ff0000", color.name(QColor::HexRgb).toUtf8());
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadOnly);
    QImageReader reader(&buffer);
    reader.setScaledSize(size);
    return reader.read();
}
