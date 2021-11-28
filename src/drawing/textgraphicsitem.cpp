#include "src/drawing/textgraphicsitem.h"

void TextGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    if (document()->isEmpty()) {
        if (document()->availableUndoSteps() > 0)
            document()->undo();
        else
            document()->redo();
        emit removeMe(this);
    }
    else
    {
        emit addMe(this);
        QGraphicsTextItem::focusOutEvent(event);
    }
}
