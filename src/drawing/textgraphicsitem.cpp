// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/drawing/textgraphicsitem.h"

#include <QFocusEvent>
#include <QTextDocument>

void TextGraphicsItem::focusOutEvent(QFocusEvent *event)
{
  if (document()->isEmpty()) {
    if (document()->availableUndoSteps() > 0) {
      document()->undo();
    } else if (document()->availableRedoSteps() > 0) {
      document()->redo();
    }
    emit removeMe(this);
  } else {
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
