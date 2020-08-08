#include "src/drawing/drawhistorystep.h"

DrawHistoryStep::DrawHistoryStep()
{

}


void DrawHistoryStep::deletePast()
{
    qDeleteAll(deletedItems);
    deletedItems.clear();
}

void DrawHistoryStep::deleteFuture()
{
    qDeleteAll(createdItems);
    createdItems.clear();
}

void DrawHistoryStep::addCreateItem(const int key, QGraphicsItem *value)
{
    createdItems[key] = value;
}

void DrawHistoryStep::addRemoveItem(const int key, QGraphicsItem *value)
{
    deletedItems[key] = value;
}

void DrawHistoryStep::purgeItem(const int key, QGraphicsItem *value)
{
    if (createdItems.value(key) == value)
        createdItems.remove(key);
    else
        deletedItems[key] = value;
}
