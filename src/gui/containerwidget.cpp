#include "src/gui/containerwidget.h"

ContainerWidget::ContainerWidget(ContainerWidget *parent) :
    QWidget(parent),
    GuiWidget(WidgetType::ContainerWidget)
{
}

const QSizeF ContainerWidget::preferedSize(const QSizeF &parent_size) const
{
    QSizeF boundary = parent_size;
    if (prefered_size.width() > 0.)
        boundary.rwidth() *= prefered_size.width();
    if (prefered_size.height() > 0.)
        boundary.rheight() *= prefered_size.height();
    if (layout() == nullptr)
        return boundary;

    qreal requested_width = 0., requested_height = 0.;
    if (layout()->expandingDirections() == Qt::Horizontal)
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(boundary);
            requested_width += widget_wants.width();
            if (requested_height < widget_wants.height())
                requested_height = widget_wants.height();
        }
    }
    else
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(boundary);
            requested_height += widget_wants.height();
            if (requested_width < widget_wants.width())
                requested_width = widget_wants.width();
        }
    }
    qDebug() << requested_width << requested_height << parent_size << this;
    if (requested_width < boundary.width())
        boundary.setWidth(requested_width);
    if (requested_height < boundary.height())
        boundary.setHeight(requested_height);
    return boundary;
}

void ContainerWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "Resize event" << event << this << layout();
    qreal requested_width = 0., requested_height = 0.;
    if (layout() == nullptr)
        return;
    QList<QSizeF> widgets_want;
    int n_free_space_widgets = 0;
    QBoxLayout *boxlayout = static_cast<QBoxLayout*>(layout());
    if (boxlayout->direction() == QBoxLayout::LeftToRight || boxlayout->direction() == QBoxLayout::RightToLeft)
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(size());
            if (widget_wants.isValid())
            {
                if (widget_wants.width() == 0.)
                    n_free_space_widgets++;
                else
                    requested_width += widget_wants.width();
                if (requested_height < widget_wants.height())
                    requested_height = widget_wants.height();
            }
            widgets_want.append(widget_wants);
        }
        qDebug() << child_widgets.length() << requested_width << requested_height << this;
        const qreal scale = requested_width > width() ? width() / requested_width : 1.;
        for (int i=0; i<child_widgets.length(); i++)
        {
            if (widgets_want[i].isValid())
            {
                if (widgets_want[i].width() != 0.)
                    child_widgets[i]->setWidth(scale * widgets_want[i].width());
            }
        }
    }
    else
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(size());
            if (widget_wants.isValid())
            {
                if (widget_wants.height() == 0.)
                    n_free_space_widgets++;
                else
                    requested_height += widget_wants.height();
                if (requested_width < widget_wants.width())
                    requested_width = widget_wants.width();
            }
            widgets_want.append(widget_wants);
        }
        qDebug() << child_widgets.length() << requested_width << requested_height << this;
        const qreal scale = requested_height > height() ? height() / requested_height : 1.;
        for (int i=0; i<child_widgets.length(); i++)
        {
            if (widgets_want[i].isValid())
            {
                if (widgets_want[i].height() != 0.)
                    child_widgets[i]->setHeight(scale * widgets_want[i].height());
            }
        }
    }
}

void ContainerWidget::setWidth(const qreal width)
{
    if (width <= 0.)
        setMaximumWidth(16777215);
    else
        setMaximumWidth(width);
}

void ContainerWidget::setHeight(const qreal height)
{
    if (height <= 0.)
        setMaximumHeight(16777215);
    else
        setMaximumHeight(height);
}
