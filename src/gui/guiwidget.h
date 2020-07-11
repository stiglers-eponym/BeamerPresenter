#ifndef GUIWIDGET_H
#define GUIWIDGET_H

/// Abstract class for distinguishing different widget types.
/// It is not a widget itself! Without casting to the correct type, all child
/// objects are useless.
class GuiWidget
{
public:
    enum WidgetType {
        InvalidType, // QWidget
        ContainerWidget, // ContainerWidget
        StackedWidget,
        Slide, // SlideView
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
        PassiveExpanding,
        PreferredAspectRatio,
        Constant,
    };

    const WidgetType type;
    GuiWidget(const WidgetType type) : type(type) {}
};

#endif // GUIWIDGET_H
