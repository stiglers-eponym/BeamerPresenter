#include "drawtoolbutton.h"

bool DrawToolButton::event(QEvent *event) noexcept
{
    switch (event->type())
    {
    case QEvent::TabletPress:
        qDebug() << "Button: set tablet tool" << tool->tool();
        emit sendTabletTool(tool);
        return true;
    case QEvent::MouseButtonRelease:
        qDebug() << "Button: set tool" << tool->tool();
        emit sendTool(tool);
    default:
        break;
    }
    return QPushButton::event(event);
}
