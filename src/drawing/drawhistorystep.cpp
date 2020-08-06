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

void DrawHistoryStep::shiftCreatedItemIndices()
{
    // All created items have indices from a list, in which the deleted
    // items were still included. They have to be shifted to get to the
    // indices in the list where removed items are really removed.
    int shift = 0;
    auto del_it = deletedItems.keyBegin();
    QList<int> keys = createdItems.keys();
    for (auto key_it = keys.cbegin(); key_it != keys.cend(); ++key_it)
    {
        while (*del_it < *key_it && del_it != deletedItems.keyEnd())
        {
            del_it++;
            shift++;
        }
        createdItems[*key_it - shift] = createdItems.take(*key_it);
    }
}
