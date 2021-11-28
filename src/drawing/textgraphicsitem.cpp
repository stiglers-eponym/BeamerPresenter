#include "src/drawing/textgraphicsitem.h"

void TextGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    if (document()->isEmpty()) {
        document()->undo();
        if (document()->isEmpty())
            emit deleteMe(this);
        else
            emit removeMe(this);
    }
    else
        QGraphicsTextItem::focusOutEvent(event);
}
