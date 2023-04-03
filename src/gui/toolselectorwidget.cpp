// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QColor>
#include <QSize>
#include <QGridLayout>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QResizeEvent>
#include "src/drawing/tool.h"
#include "src/gui/toolselectorwidget.h"
#include "src/gui/actionbutton.h"
#include "src/gui/shapeselectionbutton.h"
#include "src/gui/penstylebutton.h"
#include "src/gui/colorselectionbutton.h"
#include "src/gui/toolselectorbutton.h"
#include "src/gui/widthselectionbutton.h"
#include "src/log.h"
#include "src/names.h"

ToolSelectorWidget::ToolSelectorWidget(QWidget *parent) : QWidget(parent)
{
    setContentsMargins(1,1,1,1);
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
}

QSize ToolSelectorWidget::sizeHint() const noexcept
{
    QGridLayout *gridlayout = static_cast<QGridLayout*>(layout());
    return {gridlayout->columnCount()*22, gridlayout->rowCount()*20};
}

void ToolSelectorWidget::addButtons(const QJsonArray &full_array)
{
    QGridLayout *grid_layout = static_cast<QGridLayout*>(layout());
    for (int i=0; i<full_array.size(); i++)
    {
        const QJsonArray row = full_array[i].toArray();
        for (int j=0; j<row.size(); j++)
        {
            switch (row[j].type())
            {
            case QJsonValue::String:
            {
                const Action action = string_to_action_map.value(row[j].toString(), InvalidAction);
                if (action == InvalidAction)
                {
                    if (row[j].toString() == "shape")
                    {
                        ShapeSelectionButton *button = new ShapeSelectionButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &ShapeSelectionButton::toolChanged);
                        connect(this, &ToolSelectorWidget::updateIcons, button, &ShapeSelectionButton::updateIcon, Qt::QueuedConnection);
                        grid_layout->addWidget(button, i, j);
                    }
                    else if (row[j].toString() == "style")
                    {
                        PenStyleButton *button = new PenStyleButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &PenStyleButton::toolChanged);
                        connect(this, &ToolSelectorWidget::updateIcons, button, &PenStyleButton::updateIcon, Qt::QueuedConnection);
                        grid_layout->addWidget(button, i, j);
                    }
                }
                else
                {
                    ActionButton *button = new ActionButton(action, this);
                    connect(this, &ToolSelectorWidget::updateIcons, button, &ActionButton::updateIcon, Qt::QueuedConnection);
                    if (button->icon().isNull())
                        button->setText(row[j].toString());
                    grid_layout->addWidget(button, i, j);
                }
                break;
            }
            case QJsonValue::Array:
            {
                QJsonArray array = row[j].toArray();
                if (array.isEmpty())
                    return;
                ActionButton *button = new ActionButton(this);
                connect(this, &ToolSelectorWidget::updateIcons, button, &ActionButton::updateIcon, Qt::QueuedConnection);
                for (const auto &entry : array)
                    button->addAction(string_to_action_map.value(entry.toString(), InvalidAction));
                if (button->icon().isNull())
                    button->setText(array.first().toString());
                grid_layout->addWidget(button, i, j);
                break;
            }
            case QJsonValue::Object:
            {
                const QJsonObject obj = row[j].toObject();
                if (obj.contains("tool"))
                {
                    Tool *tool = createTool(obj, 0);
                    if (tool)
                    {
                        ToolSelectorButton *button = new ToolSelectorButton(tool, this);
                        connect(button, &ToolSelectorButton::sendTool, this, &ToolSelectorWidget::sendTool);
                        connect(this, &ToolSelectorWidget::updateIcons, button, &ToolSelectorButton::updateIcon, Qt::QueuedConnection);
                        grid_layout->addWidget(button, i, j);
                    }
                }
                else if (obj.contains("select"))
                {
                    ToolPropertyButton *button {nullptr};
                    if (obj.value("select") == "color")
                    {
                        auto *cbutton = new ColorSelectionButton(obj.value("list").toArray(), this);
                        connect(cbutton, &ColorSelectionButton::colorChanged, this, &ToolSelectorWidget::sendColor);
                        button = cbutton;
                    }
                    else if (obj.value("select") == "width")
                    {
                        auto *wbutton = new WidthSelectionButton(obj.value("list").toArray(), this);
                        connect(wbutton, &WidthSelectionButton::widthChanged, this, &ToolSelectorWidget::sendWidth);
                        button = wbutton;
                    }
                    else if(obj.value("select") == "shape")
                        button = new ShapeSelectionButton(this);
                    else if(obj.value("select") == "style")
                        button = new PenStyleButton(this);
                    if (button)
                    {
                        grid_layout->addWidget(button, i, j);
                        connect(this, &ToolSelectorWidget::sendTool, button, &ToolPropertyButton::toolChanged);
                        connect(this, &ToolSelectorWidget::updateIcons, button, &ToolPropertyButton::updateIcon, Qt::QueuedConnection);
                        connect(button, &ToolPropertyButton::sendUpdatedTool, this, &ToolSelectorWidget::updatedTool);
                    }
                }
                else
                    qWarning() << "Failed to create button" << row[j].toObject();
                break;
            }
            default:
                break;
            }
        }
    }
}

void ToolSelectorWidget::resizeEvent(QResizeEvent *event)
{
    QGridLayout *grid_layout = static_cast<QGridLayout*>(layout());
    const QSize &newsize = event->size();
    int minsize = (newsize.height() - grid_layout->verticalSpacing()) / grid_layout->rowCount() - grid_layout->verticalSpacing() - 2,
        i = 0;
    if (minsize > 10)
        while (i<grid_layout->rowCount())
            grid_layout->setRowMinimumHeight(i++, minsize);
    const int minwidth = (newsize.width() - grid_layout->horizontalSpacing()) / grid_layout->columnCount() - grid_layout->horizontalSpacing() - 6;
    if (minsize > 10)
    {
        i = 0;
        while (i<grid_layout->columnCount())
            grid_layout->setColumnMinimumWidth(i++, minwidth);
    }
    emit updateIcons();
}
