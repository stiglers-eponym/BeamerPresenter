#include "drawhistorystep.h"

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
