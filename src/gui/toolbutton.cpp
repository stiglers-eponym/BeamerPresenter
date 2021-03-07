#include "src/gui/toolbutton.h"
#include "src/preferences.h"
#include "src/gui/tooldialog.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"
#include <QTabletEvent>
#include <QMouseEvent>

ToolButton::ToolButton(Tool *tool, QWidget *parent) noexcept :
        QPushButton(parent),
        tool(NULL)
{
    setContentsMargins(0,0,0,0);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setTool(tool);
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
            if (static_cast<QInputEvent*>(event)->modifiers() == Qt::CTRL)
            {
                setTool(ToolDialog::selectTool(tool));
                debug_msg(DebugDrawing) << "Changed tool button:" << tool->tool();
                // TODO: save to GUI config
            }
            else
            {
                Tool *newtool;
                if (tool->tool() & Tool::AnyDrawTool)
                    newtool = new DrawTool(*static_cast<const DrawTool*>(tool));
                else if (tool->tool() & Tool::AnyPointingTool)
                    newtool = new PointingTool(*static_cast<const PointingTool*>(tool));
                else if (tool->tool() == Tool::TextInputTool)
                    newtool = new TextTool(*static_cast<const TextTool*>(tool));
                else
                    newtool = new Tool(*tool);

                // If tool doesn't have a device, choose a device based on the input.
                if (tool->device() == Tool::NoDevice)
                {
                    if (event->type() == QEvent::TabletRelease)
                    {
                        int device = tablet_device_to_input_device.value(static_cast<const QTabletEvent*>(event)->pointerType());
                        if (tool->tool() == Tool::Pointer && (device & (Tool::TabletPen | Tool::TabletCursor)))
                            device |= Tool::TabletNoPressure;
                        newtool->setDevice(device);
                    }
                    else if (event->type() == QEvent::MouseButtonRelease)
                    {
                        newtool->setDevice(static_cast<const QMouseEvent*>(event)->button() << 1);
                        if (tool->tool() == Tool::Pointer)
                            newtool->setDevice(newtool->device() | Tool::MouseNoButton);
                    }
                    else
                        newtool->setDevice(Tool::TouchInput);
                }
                emit sendTool(newtool);
            }
            setDown(false);
            event->accept();
            return true;
        default:
            break;
        }
    }
    return QPushButton::event(event);
}

void ToolButton::setTool(Tool *newtool)
{
    if (!newtool)
        return;
    switch (newtool->tool())
    {
    case Tool::Pen:
    {
        QPalette newpalette = palette();
        const QColor color = static_cast<DrawTool*>(newtool)->color();
        newpalette.setColor(color.lightness() > 50 ? QPalette::Button : QPalette::ButtonText, color);
        setPalette(newpalette);
        setText("pen");
        break;
    }
    case Tool::Highlighter:
    {
        QPalette newpalette = palette();
        newpalette.setColor(QPalette::Button, static_cast<DrawTool*>(newtool)->color());
        setPalette(newpalette);
        setText("highlight");
        break;
    }
    case Tool::Eraser:
        setText("eraser");
        break;
    case Tool::Pointer:
    {
        QPalette newpalette = palette();
        newpalette.setColor(QPalette::Button, static_cast<PointingTool*>(newtool)->color());
        setPalette(newpalette);
        setText("pointer");
        break;
    }
    case Tool::TextInputTool:
    {
        QPalette newpalette = palette();
        newpalette.setColor(QPalette::ButtonText, static_cast<TextTool*>(newtool)->color());
        setPalette(newpalette);
        setFont(static_cast<const TextTool*>(newtool)->font());
        setText("text");
        break;
    }
    default:
        setText(string_to_tool.key(newtool->tool()));
        break;
    }
    delete tool;
    tool = newtool;
}
