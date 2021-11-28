#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>
#include <QTextDocument>

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

    TextGraphicsItem(QGraphicsItem *parent = NULL) : QGraphicsTextItem(parent)
    {setTextInteractionFlags(Qt::TextEditorInteraction);}

    TextGraphicsItem *clone() const;

    int type() const noexcept override
    {return Type;}

    bool isEmpty() const
    {return document()->isEmpty();}

protected:
    /// emit removeMe if text is empty after this looses focus
    void focusOutEvent(QFocusEvent *event) override;

    /// disable context menu event by disabling this function
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override {}

signals:
    void addMe(QGraphicsItem *me);
    void removeMe(QGraphicsItem *me);
};

#endif // TEXTGRAPHICSITEM_H
