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

TextGraphicsItem *TextGraphicsItem::clone() const
{
    TextGraphicsItem *newitem = new TextGraphicsItem();
    newitem->setPos(pos());
    newitem->setTransform(transform());
    newitem->setFont(font());
    newitem->setDefaultTextColor(defaultTextColor());
    newitem->setDocument(document()->clone());
    return newitem;
}
