#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>

/**
 * @class TextGraphicsItem
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
    enum {Type = UserType + 3};

    TextGraphicsItem(QGraphicsItem *parent = NULL) : QGraphicsTextItem(parent) {}

    int type() const noexcept override
    {return Type;}

protected:
    /// emit deleteMe if text is empty after this looses focus
    void focusOutEvent(QFocusEvent *event) override
    {if (toPlainText().isEmpty()) emit deleteMe(this); else QGraphicsTextItem::focusOutEvent(event);}

    /// disable context menu event by disabling this function
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override {}

signals:
    void deleteMe(QGraphicsItem *me);
};

#endif // TEXTGRAPHICSITEM_H
