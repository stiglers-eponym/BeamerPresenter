#include "src/gui/toolselectorwidget.h"
#include "src/gui/actionbutton.h"
#include "src/gui/shapeselectionbutton.h"
#include "src/gui/penstylebutton.h"
#include "src/gui/colorselectionbutton.h"
#include "src/gui/toolbutton.h"
#include "src/gui/widthselectionbutton.h"
#include "src/preferences.h"
#include "src/names.h"
#include <QGridLayout>
#include <QJsonArray>

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
                        grid_layout->addWidget(button, i, j);
                    }
                    else if (row[j].toString() == "style")
                    {
                        PenStyleButton *button = new PenStyleButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &PenStyleButton::toolChanged);
                        grid_layout->addWidget(button, i, j);
                    }
                }
                else
                {
                    ActionButton *button = new ActionButton(action, this);
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
                        ToolButton *button = new ToolButton(tool, this);
                        connect(button, &ToolButton::sendTool, this, &ToolSelectorWidget::sendTool);
                        grid_layout->addWidget(button, i, j);
                    }
                }
                else if (obj.contains("select"))
                {
                    if (obj.value("select") == "color")
                    {
                        const QJsonArray array = obj.value("list").toArray();
                        ColorSelectionButton *button = new ColorSelectionButton(array, this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &ColorSelectionButton::toolChanged);
                        connect(button, &ColorSelectionButton::colorChanged, this, &ToolSelectorWidget::sendColor);
                        grid_layout->addWidget(button, i, j);
                    }
                    if (obj.value("select") == "width")
                    {
                        const QJsonArray array = obj.value("list").toArray();
                        WidthSelectionButton *button = new WidthSelectionButton(array, this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &WidthSelectionButton::toolChanged);
                        connect(button, &WidthSelectionButton::widthChanged, this, &ToolSelectorWidget::sendWidth);
                        grid_layout->addWidget(button, i, j);
                    }
                    else if(obj.value("select") == "shape")
                    {
                        ShapeSelectionButton *button = new ShapeSelectionButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &ShapeSelectionButton::toolChanged);
                        grid_layout->addWidget(button, i, j);
                    }
                    else if(obj.value("select") == "style")
                    {
                        PenStyleButton *button = new PenStyleButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &PenStyleButton::toolChanged);
                        grid_layout->addWidget(button, i, j);
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

bool ToolSelectorWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Resize)
    {
        QGridLayout *grid_layout = static_cast<QGridLayout*>(layout());
        const QSize &newsize = static_cast<QResizeEvent*>(event)->size();
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
    }
    return QWidget::event(event);
}
