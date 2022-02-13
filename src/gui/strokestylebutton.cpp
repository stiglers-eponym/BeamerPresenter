#include "src/gui/strokestylebutton.h"
#include "src/drawing/drawtool.h"

StrokeStyleButton::StrokeStyleButton(QWidget *parent) :
    ToolPropertyButton(parent)
{
    addItem("—", QVariant::fromValue(Qt::PenStyle::SolidLine));
    addItem("- -", QVariant::fromValue(Qt::PenStyle::DashLine));
    addItem(" : ", QVariant::fromValue(Qt::PenStyle::DotLine));
    addItem("- ·", QVariant::fromValue(Qt::PenStyle::DashDotLine));
    addItem("- · ·", QVariant::fromValue(Qt::PenStyle::DashDotDotLine));
}

void StrokeStyleButton::setToolProperty(Tool *tool) const
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
        static_cast<DrawTool*>(tool)->rpen().setStyle(currentData(Qt::UserRole).value<Qt::PenStyle>());
}

void StrokeStyleButton::toolChanged(Tool *tool)
{
    if (tool && tool->tool() & Tool::AnyDrawTool)
    {
        const int index = findData(QVariant::fromValue(static_cast<const DrawTool*>(tool)->pen().style()));
        if (index >= 0)
            setCurrentIndex(index);
    }
}
#include "strokestylebutton.h"
