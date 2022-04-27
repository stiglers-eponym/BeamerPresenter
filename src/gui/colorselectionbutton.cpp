#include <QColorDialog>
#include "src/gui/colorselectionbutton.h"
#include "src/gui/toolbutton.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/texttool.h"
#include "src/preferences.h"

ColorSelectionButton::ColorSelectionButton(const QJsonArray &array, QWidget *parent) :
    ToolPropertyButton(parent)
{
    QColor color;
    QSize iconsize = size();
    if (iconsize.width() > iconsize.height())
        iconsize.rwidth() = --iconsize.rheight();
    else
        iconsize.rheight() = --iconsize.rwidth();
    for (const auto &item : array)
    {
        color = QColor(item.toString());
        const QImage image = fancyIcon(preferences()->icon_path + "/tools/color.svg", iconsize, color);
        if (!image.isNull())
            addItem(QIcon(QPixmap::fromImage(image)), "", color);
        else if (color.isValid())
            addItem(color.name(), color);
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

void ColorSelectionButton::toolChanged(Tool *tool)
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
