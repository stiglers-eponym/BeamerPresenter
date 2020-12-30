#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QDebug>
#include "src/gui/flexlayout.h"
#include "src/gui/guiwidget.h"

class PixCache;

/// This whole construction might change in the future.
/// Widget for arangement of child widgets in QBoxLayout as read from
/// json configuration file.
class ContainerWidget : public QWidget, public GuiWidget
{
    Q_OBJECT

    /// Child widgets.
    QList<GuiWidget*> child_widgets;

public:
    /// Constructor: initialize QWidget and GuiWidget.
    explicit ContainerWidget(ContainerWidget *parent = nullptr) : QWidget(parent), GuiWidget(WidgetType::ContainerWidget)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Calculate preferred size based on child widgets.
    const QSizeF preferredSize(QSizeF const& parent_size) const override;

    // TODO: Probably one should use sizeHint.
    QSize sizeHint() const override
    {return layout()->sizeHint();}
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

    bool hasHeightForWidth() const noexcept override
    {return true;}

    int heightForWidth(int width) const noexcept override;

protected:
    /// Resize the widget.
    //void resizeEvent(QResizeEvent *event) override
    //{updateGeometry(); QWidget::resizeEvent(event);}

};

#endif // CONTAINERWIDGET_H
