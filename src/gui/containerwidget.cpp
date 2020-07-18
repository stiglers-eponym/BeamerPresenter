#include "src/gui/containerwidget.h"

ContainerWidget::ContainerWidget(ContainerWidget *parent) :
    QWidget(parent),
    GuiWidget(WidgetType::ContainerWidget)
{
}

const QSizeF ContainerWidget::preferedSize(const QSizeF &parent_size) const
{
    if (prefered_size.isEmpty() || layout() == nullptr)
        return prefered_size;

    QSizeF reference(parent_size.width() * prefered_size.width(), parent_size.height() * prefered_size.height());

    qreal requested_width = 0., requested_height = 0.;
    if (layout()->expandingDirections() == Qt::Horizontal)
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(reference);
            requested_width += widget_wants.width();
            if (requested_height < widget_wants.height())
                requested_height = widget_wants.height();
        }
    }
    else
    {
        for (const auto child : child_widgets)
        {
            const QSizeF widget_wants = child->preferedSize(reference);
            requested_height += widget_wants.height();
            if (requested_width < widget_wants.width())
                requested_width = widget_wants.width();
        }
    }
    return QSizeF(requested_width, requested_height);
}

void ContainerWidget::resizeEvent(QResizeEvent *event)
{
    qDebug() << "Resize event" << event << this << layout();
    qreal requested_width = 0., requested_height = 0.;
    if (layout() == nullptr)
        return;
    QVector<QSizeF> widgets_want;
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
        const qreal per_free_widget = (1. - scale * requested_width) / n_free_space_widgets;
        for (int i=0; i<child_widgets.length(); i++)
        {
            if (widgets_want[i].isValid())
            {
                if (widgets_want[i].width() == 0.)
                    child_widgets[i]->setWidth(per_free_widget);
                else
                    child_widgets[i]->setWidth(scale * widgets_want[i].width());
            }
        }
        setHeight(requested_height > height() ? height() : requested_height);
    }
    // TODO: implement vertical layout
}

void ContainerWidget::setWidth(const qreal width)
{
    setMinimumWidth(width);
}

void ContainerWidget::setHeight(const qreal height)
{
    setMinimumHeight(height);
}
