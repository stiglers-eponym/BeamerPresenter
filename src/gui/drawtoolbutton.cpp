#include "drawtoolbutton.h"

DrawToolButton::DrawToolButton(Tool *tool, QWidget *parent) noexcept :
        QPushButton(parent),
        tool(tool)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

bool DrawToolButton::event(QEvent *event) noexcept
{
    if (tool)
    {
        switch (event->type())
        {
        case QEvent::TouchBegin:
            event->accept();
            setDown(true);
            return true;
        case QEvent::TouchEnd:
        {
            const QTouchEvent *touchevent = static_cast<const QTouchEvent*>(event);
            if (touchevent->touchPoints().size() != 1 || !rect().contains(touchevent->touchPoints().first().pos().toPoint()))
            {
                setDown(false);
                return false;
            }
        }
            [[clang::fallthrough]];
        case QEvent::TabletRelease:
        case QEvent::MouseButtonRelease:
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
            if (event->type() == QEvent::TabletRelease)
                newtool->setDevice(tablet_device_to_input_device.value(static_cast<const QTabletEvent*>(event)->pointerType()));
            else if (event->type() == QEvent::MouseButtonRelease)
                newtool->setDevice(mouse_to_input_device.value(static_cast<const QMouseEvent*>(event)->button()));
            else
                newtool->setDevice(TouchInput);
            emit sendTool(newtool);
            setDown(false);
            event->accept();
            return true;
        default:
            break;
        }
    }
    return QPushButton::event(event);
}
