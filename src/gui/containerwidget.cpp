#include "src/gui/containerwidget.h"

int ContainerWidget::heightForWidth(int width) const noexcept
{
    int height = 0;
    switch (static_cast<QBoxLayout*>(layout())->direction())
    {
    case QBoxLayout::LeftToRight:
    case QBoxLayout::RightToLeft:
        for (const auto child : qAsConst(child_widgets))
            height = std::max(height, child->heightForWidth(width/child_widgets.size()));
        break;
    case QBoxLayout::TopToBottom:
    case QBoxLayout::BottomToTop:
        for (const auto child : qAsConst(child_widgets))
            height += child->heightForWidth(width/child_widgets.size());
        break;
    }
    qDebug() << width << height;
    return height;
}
