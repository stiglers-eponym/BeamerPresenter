// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QColor>
#include <QColorDialog>
#include <QPixmap>
#include <QIcon>
#include <QImage>
#include <QSize>
#include <QJsonArray>
#include <QJsonValue>
#include <QColor>
#include "src/gui/colorselectionbutton.h"
#include "src/gui/toolbutton.h"
#include "src/gui/tooliconengine.h"
#include "src/drawing/tool.h"
#include "src/preferences.h"

ColorSelectionButton::ColorSelectionButton(const QJsonArray &array, QWidget *parent) :
    ToolPropertyButton(parent)
{
    setToolTip(tr("select color for tools"));
    const QSize rendersize {64, 64};
    QColor color;
    for (const auto &item : array)
    {
        color = QColor(item.toString());
        if (!color.isValid())
            continue;
        QIcon icon(new ToolIconEngine(preferences()->icon_path + "/tools/color.svg", color.rgba(), 0));
        if (icon.isNull())
            addItem(color.name(), color);
        else
            addItem(icon, "", color);
    }
    addItem("?", QColor());
}

void ColorSelectionButton::setToolProperty(Tool *tool) const
{
    QColor color = currentData(Qt::UserRole).value<QColor>();
    if (!color.isValid())
        color = QColorDialog::getColor(Qt::black, parentWidget(), tr("Tool color"), QColorDialog::ShowAlphaChannel);
    if (tool && !(tool->tool() & Tool::AnySelectionTool))
        tool->setColor(color);
    emit colorChanged(color);
}

void ColorSelectionButton::toolChanged(const Tool *tool)
{
    if (tool)
    {
        const int idx = findData(tool->color());
        if (idx >= 0)
            setCurrentIndex(idx);
        else
            setCurrentIndex(count()-1);
    }
}
