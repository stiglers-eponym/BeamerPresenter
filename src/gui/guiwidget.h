#ifndef GUIWIDGET_H
#define GUIWIDGET_H

#include <QSizeF>

/// This whole construction might change in the future.
/// Abstract class for distinguishing different widget types.
/// Inheriting classes should also inherit from QWidget.
/// This is not a QWidget itself!
class GuiWidget
{
protected:
    /// Prefered size of the widget inside the given geometry of its parent
    /// widget.
    QSizeF preferred_size;

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

    /// Set preferred size
    void setPreferredSize(const QSizeF& size) noexcept
    {preferred_size = size;}

    /// Get preferred size based on parent_size.
    virtual const QSizeF preferredSize(QSizeF const& parent_size) const
    {return preferred_size;}

    /// Set (maximum) widget width.
    virtual void setWidth(const qreal width) = 0;

    /// Set (maximum) widget height.
    virtual void setHeight(const qreal height) = 0;

    virtual int heightForWidth(int width) const noexcept = 0;
};

#endif // GUIWIDGET_H
