#include "src/drawing/drawhistorystep.h"

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
