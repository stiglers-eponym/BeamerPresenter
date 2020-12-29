#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QBoxLayout>
#include "src/gui/guiwidget.h"

class PixCache;

/// This whole construction might change in the future.
/// Widget for flexible arangement of child widgets as read from
/// json configuration file.
class ContainerWidget : public QWidget, public GuiWidget
{
    Q_OBJECT

    /// Child widgets.
    QList<GuiWidget*> child_widgets;

public:
    /// Constructor: initialize QWidget and GuiWidget.
    explicit ContainerWidget(ContainerWidget *parent = nullptr) : QWidget(parent), GuiWidget(WidgetType::ContainerWidget) {}

    /// Calculate preferred size based on child widgets.
    const QSizeF preferredSize(QSizeF const& parent_size) const override;

    // TODO: Probably one should use sizeHint.
    //QSize sizeHint() const override
    //{return preferredSize(size()).toSize();}

    /// Set (maximum) widget width.
    void setWidth(const qreal width) override
    {setMaximumWidth(width >= 0. ? width : 16777215);}

    /// Set (maximum) widget height.
    void setHeight(const qreal height) override
    {setMaximumHeight(height >= 0. ? height : 16777215);}

    /// Add child widget.
    void addGuiWidget(GuiWidget* widget)
    {child_widgets.append(widget);}

protected:
    /// Resize the widget.
    void resizeEvent(QResizeEvent *event) override;

};

#endif // CONTAINERWIDGET_H
