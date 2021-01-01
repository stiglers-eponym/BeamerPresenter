#include "drawtoolbutton.h"

DrawToolButton::DrawToolButton(Tool *tool, QWidget *parent) noexcept :
        QPushButton(parent),
        tool(tool)
{
    setFocusPolicy(Qt::NoFocus);
}

void DrawToolButton::tabletEvent(QTabletEvent *event) noexcept
{
    if (event->type() == QTabletEvent::TabletRelease && tool)
    {
        Tool *newtool;
        switch(tool->tool())
        {
        case Pen:
        case Highlighter:
            newtool = new DrawTool(*static_cast<const DrawTool*>(tool));
            break;
        default:
            newtool = new Tool(*tool);
            break;
        }
        newtool->setDevice(tablet_device_to_input_device.value(event->pointerType()));
        emit sendTool(newtool);
    }
    event->ignore();
}

void DrawToolButton::mouseReleaseEvent(QMouseEvent *event) noexcept
{
    if (event->type() == QMouseEvent::MouseButtonRelease && tool)
    {
        Tool *newtool;
        switch(tool->tool())
        {
        case Pen:
        case Highlighter:
            newtool = new DrawTool(*static_cast<const DrawTool*>(tool));
            break;
        default:
            newtool = new Tool(*tool);
            break;
        }
        newtool->setDevice(mouse_to_input_device.value(event->button()));
        emit sendTool(newtool);
        event->accept();
    }
    QPushButton::mouseReleaseEvent(event);
}
