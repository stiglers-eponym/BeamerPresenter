#include "toolselectorwidget.h"

ToolSelectorWidget::ToolSelectorWidget(QWidget *parent) : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
}

QSize ToolSelectorWidget::sizeHint() const noexcept
{
    QGridLayout *gridlayout = static_cast<QGridLayout*>(layout());
    return {gridlayout->columnCount()*30, gridlayout->rowCount()*10};
}

void ToolSelectorWidget::addActionButton(const int i, const int j, const QString &string)
{
    ActionButton *button = new ActionButton(string_to_action_map.value(string, InvalidAction), this);
    button->setText(string);
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
    connect(button, &ActionButton::sendAction, this, &ToolSelectorWidget::sendAction);
}

void ToolSelectorWidget::addToolButton(const int i, const int j, Tool *tool)
{
    ToolButton *button = new ToolButton(tool, this);
    connect(button, &ToolButton::sendTool, this, &ToolSelectorWidget::sendTool);
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
}
