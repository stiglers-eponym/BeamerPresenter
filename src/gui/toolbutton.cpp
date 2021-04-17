#include "src/gui/toolbutton.h"
#include "src/preferences.h"
#include "src/gui/tooldialog.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"
#include <QTabletEvent>
#include <QMouseEvent>
#include <QBuffer>
#include <QImageReader>

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
    if (tool_icon_names.contains(newtool->tool()))
    {
        QColor color;
        if (newtool->tool() & Tool::AnyDrawTool)
            color = static_cast<DrawTool*>(newtool)->color();
        else if (newtool->tool() & Tool::AnyPointingTool)
            color = static_cast<PointingTool*>(newtool)->color();
        else if (newtool->tool() == Tool::TextInputTool)
        {
            color = static_cast<TextTool*>(newtool)->color();
            if (!color.isValid())
                color = Qt::black;
        }
        QIcon icon;
        const QString filename = preferences()->icon_path + tool_icon_names[newtool->tool()];
        if (color.isValid())
        {
            const QImage image = fancyIcon(filename, iconSize(), color);
            if (!image.isNull())
                icon = QIcon(QPixmap::fromImage(image));
        }
        if (icon.isNull())
            icon = QIcon(filename);
        if (icon.isNull())
            setText(string_to_tool.key(newtool->tool()));
        else
            setIcon(icon);
    }
    else
        setText(string_to_tool.key(newtool->tool()));
    delete tool;
    tool = newtool;
    setToolTip(tool_to_description.value(tool->tool()));
}

const QImage fancyIcon(const QString &filename, const QSize &size, const QColor &color)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return QImage();
    QByteArray data = file.readAll();
    if (data.isEmpty())
        return QImage();
    data.replace("#ff0000", color.name(QColor::HexRgb).toUtf8());
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadOnly);
    QImageReader reader(&buffer);
    reader.setScaledSize(size);
    return reader.read();
}
