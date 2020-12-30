#ifndef GUIWIDGET_H
#define GUIWIDGET_H

#include <QSizeF>

/// This whole construction might change in the future.
/// Abstract class for distinguishing different widget types.
/// Inheriting classes should also inherit from QWidget.
/// This is not a QWidget itself!
class GuiWidget
{
public:
    /// Type of the widget.
    enum WidgetType {
        InvalidType = 0, // QWidget
        ContainerWidget, // ContainerWidget (QBoxLayout)
        StackedWidget, // StackedWidget (QStackedLayout)
        TabedWidget, // TabedWidget (QTabedLayout)
        Slide, // SlideView (QGraphicsView)
        Overview,
        TOC,
        Notes,
        Button,
        ToolSelector,
        Settings,
        Clock,
        Timer,
        SlideNumber,
    };
    enum SizePolicy {
        /// preferes to have a fixed aspect ratio
        FixedAspect,
        FixedHeight,
        FixedWidth,
        FixedSize,
        /// has a minimum size
        Expanding,
    };

    const WidgetType type;

    /// Constructor: only initialize type.
    GuiWidget(const WidgetType type) noexcept : type(type) {}

    /// Set (maximum) widget width.
    virtual void setWidth(const qreal width) = 0;

    /// Set (maximum) widget height.
    virtual void setHeight(const qreal height) = 0;

    virtual int heightForWidth(int width) const noexcept = 0;
};

#endif // GUIWIDGET_H
