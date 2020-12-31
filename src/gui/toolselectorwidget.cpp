#include "toolselectorwidget.h"

ToolSelectorWidget::ToolSelectorWidget(QWidget *parent) : QWidget(parent)
{
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
}

QSize ToolSelectorWidget::sizeHint() const noexcept
{
    QGridLayout *gridlayout = static_cast<QGridLayout*>(layout());
    return {gridlayout->columnCount()*30, gridlayout->rowCount()*15};
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
    DrawToolButton *button = new DrawToolButton(tool, this);
    switch (tool->tool())
    {
    case Pen:
    {
        QPalette newpalette = button->palette();
        const QColor color = static_cast<DrawTool*>(tool)->color();
        newpalette.setColor(color.lightness() > 50 ? QPalette::Button : QPalette::ButtonText, color);
        button->setPalette(newpalette);
        button->setText("pen");
        break;
    }
    case Highlighter:
    {
        QPalette newpalette = button->palette();
        newpalette.setColor(QPalette::Button, static_cast<DrawTool*>(tool)->color());
        button->setPalette(newpalette);
        button->setText("higlight");
        break;
    }
    case Eraser:
        button->setText("eraser");
        break;
    default:
        button->setText(QString::number(tool->tool()));
        break;
    }
    connect(button, &DrawToolButton::sendTool, this, &ToolSelectorWidget::sendTool);
    connect(button, &DrawToolButton::sendTabletTool, this, &ToolSelectorWidget::sendTabletTool);
    static_cast<QGridLayout*>(layout())->addWidget(button, i, j);
}
