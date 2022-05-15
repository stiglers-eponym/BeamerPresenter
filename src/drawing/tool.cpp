#include "src/drawing/tool.h"
#include "src/preferences.h"

int tablet_event_to_input_device(const QTabletEvent* event)
{
    debug_verbose(DebugDrawing, event << event->pointerType() << event->buttons());
    if (event->pressure() <= 0 && event->type() != QEvent::TabletRelease)
        return Tool::TabletHover;
    switch (event->pointerType())
    {
#if (QT_VERSION_MAJOR >= 6)
    case QPointingDevice::PointerType::Pen:
        if (event->buttons() & Qt::MiddleButton)
            return Tool::TabletMod;
        else
            return Tool::TabletPen;
    case QPointingDevice::PointerType::Eraser:
        return Tool::TabletEraser;
    case QPointingDevice::PointerType::Cursor:
        return Tool::TabletCursor;
    case QPointingDevice::PointerType::Generic:
        return Tool::MouseNoButton;
    case QPointingDevice::PointerType::Finger:
        return Tool::TouchInput;
#else
    case QTabletEvent::Pen:
        return Tool::TabletPen;
    case QTabletEvent::Eraser:
        return Tool::TabletEraser;
    case QTabletEvent::Cursor:
        return Tool::TabletCursor;
    case QTabletEvent::UnknownPointer:
        return Tool::TabletOther;
#endif
    default:
        return Tool::TabletOther;
    }
}
