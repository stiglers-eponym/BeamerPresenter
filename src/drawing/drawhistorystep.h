#ifndef DRAWHISTORYSTEP_H
#define DRAWHISTORYSTEP_H

#include <QMap>
#include <QGraphicsItem>

/// One single step in the history of drawing.
/// Save deleted and added GraphicsItems and their position (by their index).
class DrawHistoryStep
{
private:
    /// Newly created items with their index after the history step.
    QMap<int, QGraphicsItem*> createdItems;
    /// Deleted items with their indices before the history step.
    QMap<int, QGraphicsItem*> deletedItems;

public:
    DrawHistoryStep();
    void addCreateItem(int key, QGraphicsItem* value) {createdItems[key] = value;}
    void addRemoveItem(int key, QGraphicsItem* value) {deletedItems[key] = value;}
    const QMap<int, QGraphicsItem*>& getCreatedItems() const {return createdItems;}
    const QMap<int, QGraphicsItem*>& getDeletedItems() const {return deletedItems;}
    int numberRemoved() const {return deletedItems.size();}
    int numberCreated() const {return createdItems.size();}
    void deletePast();
    void deleteFuture();
    /// Shift indices of created items which were inserted in eraserMicroStep.
    /// TODO: explain
    void shiftCreatedItemIndices();
};

#endif // DRAWHISTORYSTEP_H
