#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>
#include <QTextDocument>

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
    enum {Type = UserType + 5};

    /// almost trivial constructor
    TextGraphicsItem(QGraphicsItem *parent = NULL) : QGraphicsTextItem(parent)
    {setTextInteractionFlags(Qt::TextEditorInteraction);}

    /// Copy/clone this item.
    TextGraphicsItem *clone() const;

    /// QGraphicsItem type.
    int type() const noexcept override
    {return Type;}

    /// Check if text is empty.
    bool isEmpty() const
    {return document()->isEmpty();}

protected:
    /// emit removeMe if text is empty after this looses focus
    void focusOutEvent(QFocusEvent *event) override;

    /// disable context menu event by disabling this function
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override {}

signals:
    /// Add this to history management.
    /// Emitted when focus is removed and this is not empty.
    void addMe(QGraphicsItem *me);
    /// Add history step that removes this.
    /// Emitted when all text is deleted.
    void removeMe(QGraphicsItem *me);
};

#endif // TEXTGRAPHICSITEM_H
