#ifndef GUIWIDGET_H
#define GUIWIDGET_H

#include <QSizeF>

/// Abstract class for distinguishing different widget types.
/// It is not a widget itself! Without casting to the correct type, all child
/// objects are useless.
class GuiWidget
{
protected:
    QSizeF prefered_size;
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
    void setPreferedSize(const QSizeF& size) {prefered_size = size;}
    virtual const QSizeF preferedSize(QSizeF const& parent_size) const {return prefered_size;}
    virtual void setWidth(const qreal width) = 0;
    virtual void setHeight(const qreal height) = 0;
};

#endif // GUIWIDGET_H
