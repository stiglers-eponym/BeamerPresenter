#include "toolbutton.h"

ToolButton::ToolButton(Tool *tool, QWidget *parent) noexcept :
        QPushButton(parent),
        tool(tool)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
}

bool ToolButton::event(QEvent *event) noexcept
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
            if (tool->tool() & AnyDrawTool)
                newtool = new DrawTool(*static_cast<const DrawTool*>(tool));
            else if (tool->tool() & AnyPointingTool)
                newtool = new PointingTool(*static_cast<const PointingTool*>(tool));
            else
                newtool = new Tool(*tool);

            if (event->type() == QEvent::TabletRelease)
                newtool->setDevice(tablet_device_to_input_device.value(static_cast<const QTabletEvent*>(event)->pointerType()));
            else if (event->type() == QEvent::MouseButtonRelease)
            {
                newtool->setDevice(static_cast<const QMouseEvent*>(event)->button() << 1);
                if (tool->tool() == Pointer)
                    newtool->setDevice(newtool->device() | MouseNoButton);
            }
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
