#include "src/gui/toolselectorwidget.h"
#include "src/gui/actionbutton.h"
#include "src/gui/shapeselectionbutton.h"
#include "src/gui/strokestylebutton.h"
#include "src/gui/colorselectionbutton.h"
#include "src/gui/toolbutton.h"
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
    return {gridlayout->columnCount()*30, gridlayout->rowCount()*10};
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
                        StrokeStyleButton *button = new StrokeStyleButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &StrokeStyleButton::toolChanged);
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
                        StrokeStyleButton *button = new StrokeStyleButton(this);
                        connect(this, &ToolSelectorWidget::sendTool, button, &StrokeStyleButton::toolChanged);
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
