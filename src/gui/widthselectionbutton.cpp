#include "src/gui/widthselectionbutton.h"
#include "src/drawing/drawtool.h"
#include <QJsonArray>

WidthSelectionButton::WidthSelectionButton(const QJsonArray &array, QWidget *parent) :
    ToolPropertyButton(parent)
{
    QSize iconsize = size();
    if (iconsize.width() > iconsize.height())
        iconsize.rwidth() = --iconsize.rheight();
    else
        iconsize.rheight() = --iconsize.rwidth();
    qreal width;
    for (const auto &item : array)
    {
        width = item.toDouble(-1.);
        if (width > 0.)
            addItem(QString::number(width, 'g', 2), width);
    }
}

void WidthSelectionButton::setToolProperty(Tool *tool) const
{
    if (!(tool && (tool->tool() & Tool::AnyDrawTool || tool->tool() & Tool::AnySelectionTool)))
        return;
    const qreal width = currentData(Qt::UserRole).value<qreal>();
    if (width <= 0.)
        return;
    if (tool->tool() & Tool::AnySelectionTool)
        emit widthChanged(width);
    else
        static_cast<DrawTool*>(tool)->setWidth(width);
}

void WidthSelectionButton::toolChanged(Tool *tool)
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
    {
        const int idx = findData(static_cast<const DrawTool*>(tool)->width());
        if (idx >= 0)
            setCurrentIndex(idx);
    }
}
