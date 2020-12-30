#include "src/gui/stackedwidget.h"

QSize StackedWidget::sizeHint() const noexcept
{
    QSize size(0,0);
    int i = 0;
    while (i < count())
        size = size.expandedTo(widget(i++)->sizeHint());
    return size;
}
