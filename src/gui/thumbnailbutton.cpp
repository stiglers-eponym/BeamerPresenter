#include "thumbnailbutton.h"

void ThumbnailButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit sendNavigationSignal(page);
        event->accept();
    }
}
