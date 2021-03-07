#include "src/gui/toolselectorwidget.h"
#include "src/gui/actionbutton.h"
#include "src/gui/toolbutton.h"
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
    gridlayout->setMargin(0);
    return {gridlayout->columnCount()*30, gridlayout->rowCount()*10};
}

void ToolSelectorWidget::addActionButton(const int i, const int j, const QString &string)
{
    ActionButton *button = new ActionButton(string_to_action_map.value(string, InvalidAction), this);
    button->setText(string);
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
    connect(button, &ActionButton::sendAction, this, &ToolSelectorWidget::sendAction);
}

void ToolSelectorWidget::addActionButton(const int i, const int j, const QJsonArray &array)
{
    if (array.isEmpty())
        return;
    ActionButton *button = new ActionButton(this);
    for (const auto &entry : array)
        button->addAction(string_to_action_map.value(entry.toString(), InvalidAction));
    button->setText(array.first().toString());
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
    connect(button, &ActionButton::sendAction, this, &ToolSelectorWidget::sendAction);
}

void ToolSelectorWidget::addToolButton(const int i, const int j, Tool *tool)
{
    ToolButton *button = new ToolButton(tool, this);
    connect(button, &ToolButton::sendTool, this, &ToolSelectorWidget::sendTool);
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
}
