// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>
#include <QTextDocument>
#include "src/config.h"
#include "src/enumerates.h"

class QGraphicsSceneContextMenuEvent;
class QFocusEvent;

/**
 * @brief QGraphicsTextItem with disabled context menu
 *
 * There seems to be some bug related to the context menu of the
 * QGraphicsTextItem, which leads to segmentation faults when right-clicking
 * on a QGraphicsTextItem. This is avoided by using this class instead of
 * QGraphicsTextItem.
 */
class TextGraphicsItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    /// QGraphicsItem type for this subclass
    enum { Type = UserType + TextGraphicsItemType };

    /// almost trivial constructor
    TextGraphicsItem(QGraphicsItem *parent = nullptr) : QGraphicsTextItem(parent)
    {
        setFlags(QGraphicsItem::ItemIsSelectable);
        setTextInteractionFlags(Qt::TextEditorInteraction);
    }

    /// @return copy this item
    TextGraphicsItem *clone() const;

    /// @return custom QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Check if text is empty.
    bool isEmpty() const noexcept
    {return document()->isEmpty();}

protected:
    /// Called when this looses focus.
    /// Notify PathContainer of new or removed text items.
    /// @see removeMe()
    /// @see addMe()
    void focusOutEvent(QFocusEvent *event) override;

    /// disable context menu event by disabling this function
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override {}

signals:
    /// Tell PathContainer to add this to history.
    /// Emitted when focus is removed and this is not empty.
    /// @see focusOutEvent()
    void addMe(QGraphicsItem *me);

    /// Tell PathContainer to remove this.
    /// Add history step in PathContainer that removes this.
    /// Emitted when all text is deleted and this looses focus.
    /// @see focusOutEvent()
    void removeMe(QGraphicsItem *me);
};

#endif // TEXTGRAPHICSITEM_H
