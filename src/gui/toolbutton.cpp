#include <QTabletEvent>
#include <QMouseEvent>
#include <QBuffer>
#include <QImageReader>
#include "src/gui/toolbutton.h"
#include "src/preferences.h"
#include "src/gui/tooldialog.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/selectiontool.h"

ToolButton::ToolButton(Tool *tool, QWidget *parent) noexcept :
        QToolButton(parent),
        tool(NULL)
{
    setMinimumSize(12, 12);
    setIconSize({32,32});
    setContentsMargins(0,0,0,0);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
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
#if (QT_VERSION_MAJOR >= 6)
            if (touchevent->points().size() != 1 || !rect().contains(touchevent->points().first().position().toPoint()))
#else
            if (touchevent->touchPoints().size() != 1 || !rect().contains(touchevent->touchPoints().first().pos().toPoint()))
#endif
            {
                setDown(false);
                return false;
            }
        }
            [[clang::fallthrough]];
        case QEvent::TabletMove:
        case QEvent::TabletPress:
        case QEvent::MouseButtonPress:
            if (static_cast<QInputEvent*>(event)->modifiers() == Qt::CTRL)
            {
                setTool(ToolDialog::selectTool(tool));
                debug_msg(DebugDrawing, "Changed tool button:" << tool->tool());
                // TODO: save to GUI config
            }
            else
            {
                // If tool doesn't have a device, choose a device based on the input.
                int device = tool->device();
                if (event->type() == QEvent::TabletMove || event->type() == QEvent::TabletPress)
                {
                    const QTabletEvent *tablet_event = static_cast<const QTabletEvent*>(event);
                    if (tablet_event->pressure() <= 0 || tablet_event->pressure() == 1)
                        break;
                    if (device == Tool::NoDevice)
                    {
                        device = tablet_event_to_input_device(tablet_event);
                        if (tool->tool() == Tool::Pointer && (device & (Tool::TabletPen | Tool::TabletCursor)))
                            device |= Tool::TabletHover;
                    }
                }
                else if (event->type() == QEvent::MouseButtonPress && device == Tool::NoDevice)
                {
                    device = static_cast<const QMouseEvent*>(event)->button() << 1;
                    if (tool->tool() == Tool::Pointer)
                        device |= Tool::MouseNoButton;
                }
                else if (event->type() == QEvent::TouchEnd && device == Tool::NoDevice)
                    device = Tool::TouchInput;

                Tool *newtool;
                if (tool->tool() & Tool::AnyDrawTool)
                    newtool = new DrawTool(*static_cast<const DrawTool*>(tool));
                else if (tool->tool() & Tool::AnyPointingTool)
                    newtool = new PointingTool(*static_cast<const PointingTool*>(tool));
                else if (tool->tool() & Tool::AnySelectionTool)
                    newtool = new SelectionTool(*static_cast<const SelectionTool*>(tool));
                else if (tool->tool() == Tool::TextInputTool)
                    newtool = new TextTool(*static_cast<const TextTool*>(tool));
                else
                    newtool = new Tool(*tool);
                newtool->setDevice(device);
                emit sendTool(newtool);
            }
            setDown(false);
            event->accept();
            return true;
        case QEvent::Resize:
            setTool(tool);
            break;
        default:
            break;
        }
    }
    return QToolButton::event(event);
}

void ToolButton::setTool(Tool *newtool)
{
    if (!newtool)
        return;
    QColor color;
    QString iconname = string_to_tool.key(newtool->tool());
    iconname.replace(' ', '-');
    if (newtool->tool() & Tool::AnyDrawTool)
    {
        const DrawTool *drawtool = static_cast<const DrawTool*>(newtool);
        if (drawtool->shape() != DrawTool::Freehand)
        {
            if (drawtool->tool() == Tool::FixedWidthPen && drawtool->shape() != DrawTool::Recognize)
                iconname = "pen";
            iconname += "-";
            iconname += string_to_shape.key(drawtool->shape());
        }
        if (drawtool->brush().style() != Qt::NoBrush && drawtool->shape() != DrawTool::Arrow && drawtool->shape() != DrawTool::Line)
            iconname += "-filled";
        color = drawtool->color();
    }
    else
        color = newtool->color();
    if (!color.isValid())
        color = Qt::black;
    const QString filename = preferences()->icon_path + "/tools/" + iconname + ".svg";
    QSize newsize = size();
    setIconSize(newsize);
    QIcon icon;
    if (color.isValid())
    {
        if (newsize.height() > newsize.width())
            newsize.rheight() = newsize.width();
        else
            newsize.rwidth() = newsize.height();
        const QImage image = fancyIcon(filename, newsize, color);
        if (!image.isNull())
            icon = QIcon(QPixmap::fromImage(image));
    }
    if (icon.isNull())
        icon = QIcon(filename);
    if (icon.isNull())
        setText(string_to_tool.key(newtool->tool()));
    else
        setIcon(icon);
    if (tool != newtool)
    {
        delete tool;
        tool = newtool;
        setToolTip(tr(tool_to_description.value(tool->tool())));
    }
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
