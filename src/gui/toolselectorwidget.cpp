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
    for (int row_index=0; row_index<full_array.size(); row_index++)
    {
        const QJsonArray row = full_array[row_index].toArray();
        for (int column_index=0; column_index<row.size(); column_index++)
        {
            switch (row[column_index].type())
            {
            case QJsonValue::String:
            {
                const QString &name = row[column_index].toString();
                const Action action = string_to_action_map.value(name, InvalidAction);
                if (action == InvalidAction)
                    initializeToolPropertyButton(name, {}, row_index, column_index);
                else
                {
                    ActionButton *button = new ActionButton(action, this);
                    connect(this, &ToolSelectorWidget::updateIcons, button, &ActionButton::updateIcon, Qt::QueuedConnection);
                    if (button->icon().isNull())
                        button->setText(name);
                    grid_layout->addWidget(button, row_index, column_index);
                }
                break;
            }
            case QJsonValue::Array:
            {
                QJsonArray array = row[column_index].toArray();
                if (array.isEmpty())
                    break;
                ActionButton *button = new ActionButton(this);
                connect(this, &ToolSelectorWidget::updateIcons, button, &ActionButton::updateIcon, Qt::QueuedConnection);
                for (const auto &entry : array)
                    button->addAction(string_to_action_map.value(entry.toString(), InvalidAction));
                if (button->icon().isNull())
                    button->setText(array.first().toString());
                grid_layout->addWidget(button, row_index, column_index);
                break;
            }
            case QJsonValue::Object:
            {
                const QJsonObject obj = row[column_index].toObject();
                if (obj.contains("tool"))
                {
                    Tool *tool = createTool(obj, 0);
                    if (tool)
                    {
                        ToolSelectorButton *button = new ToolSelectorButton(tool, this);
                        connect(button, &ToolSelectorButton::sendTool, this, &ToolSelectorWidget::sendTool);
                        connect(this, &ToolSelectorWidget::updateIcons, button, &ToolSelectorButton::updateIcon, Qt::QueuedConnection);
                        grid_layout->addWidget(button, row_index, column_index);
                    }
                }
                else if (obj.contains("select"))
                    initializeToolPropertyButton(obj.value("select").toString(), obj.value("list").toArray(), row_index, column_index);
                else
                    qWarning() << "Failed to create button" << row[column_index].toObject();
                break;
            }
            default:
                break;
            }
        }
    }
}

void ToolSelectorWidget::initializeToolPropertyButton(const QString &type, const QJsonArray &list, const int row, const int column)
{
    ToolPropertyButton *button {nullptr};
    if (type == "color")
    {
        auto *cbutton = new ColorSelectionButton(list, this);
        connect(cbutton, &ColorSelectionButton::colorChanged, this, &ToolSelectorWidget::sendColor);
        button = cbutton;
    }
    else if (type == "width")
    {
        auto *wbutton = new WidthSelectionButton(list, this);
        connect(wbutton, &WidthSelectionButton::widthChanged, this, &ToolSelectorWidget::sendWidth);
        button = wbutton;
    }
    else if(type == "shape")
        button = new ShapeSelectionButton(this);
    else if(type == "style")
        button = new PenStyleButton(this);
    if (button)
    {
        static_cast<QGridLayout*>(layout())->addWidget(button, row, column);
        connect(this, &ToolSelectorWidget::sendTool, button, &ToolPropertyButton::toolChanged);
        connect(this, &ToolSelectorWidget::updateIcons, button, &ToolPropertyButton::updateIcon, Qt::QueuedConnection);
        connect(button, &ToolPropertyButton::sendUpdatedTool, this, &ToolSelectorWidget::updatedTool);
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
