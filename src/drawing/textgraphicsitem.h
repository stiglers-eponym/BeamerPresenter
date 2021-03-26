#ifndef TEXTGRAPHICSITEM_H
#define TEXTGRAPHICSITEM_H

#include <QGraphicsTextItem>

class TextGraphicsItem : public QGraphicsTextItem
{
    Q_OBJECT

public:
    enum {Type = UserType + 3};

    TextGraphicsItem(QGraphicsItem *parent = NULL) : QGraphicsTextItem(parent) {}

    int type() const noexcept override
    {return Type;}

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent*) override {}
};

#endif // TEXTGRAPHICSITEM_H
