#include "src/drawing/pathcontainer.h"

PathContainer::PathContainer(QObject *parent) : QObject(parent)
{

}

PathContainer::~PathContainer()
{
    clearHistory();
    // This is dangerous: check which paths are owned by QGraphicsScene.
    qDeleteAll(paths);
    paths.clear();
}

bool PathContainer::undo(QGraphicsScene *scene)
{
    // Check whether a further entry in history exists.
    if (history.length() - inHistory < 1)
        return false;

    // Mark that we moved back in history.
    ++inHistory;

    // First remove newly created items.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &removeItems = history[history.length()-inHistory]->getCreatedItems();
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = removeItems.constEnd(); it-- != removeItems.constBegin() - 1;)
    {
        paths.removeAt(it.key());
        // TODO: check if it is necessary to hide items explicitly.
        if (scene != nullptr)
            scene->removeItem(it.value());
    }

    // Restore old items.
    // Get the old items from history.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->getDeletedItems();
    for (auto it = oldItems.constBegin(); it != oldItems.constEnd(); ++it)
    {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene != nullptr)
        {
            scene->addItem(it.value());
            if (it.key() + 1 < paths.length())
                it.value()->stackBefore(paths[it.key() + 1]);
        }
    }

    // TODO: update the screen.
    return true;
}

bool PathContainer::redo(QGraphicsScene *scene)
{
    // First check whether there is something to redo in history.
    if (inHistory < 1)
        return false;

    // First remove items which were deleted in this step.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->getDeletedItems();
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = oldItems.constEnd(); it-- != oldItems.constBegin() - 1;) {
        paths.removeAt(it.key());
        // TODO: check if it is necessary to hide items explicitly.
        if (scene != nullptr)
            scene->removeItem(it.value());
    }

    // Restore newly created items.
    // Get the new items from history.
    const QMap<int, QGraphicsItem*> &newItems = history[history.length()-inHistory]->getCreatedItems();
    for (auto it = newItems.constBegin(); it != newItems.constEnd(); ++it) {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene != nullptr) {
            scene->addItem(it.value());
            if (it.key() + 1 < paths.length())
                it.value()->stackBefore(paths[it.key() + 1]);
        }
    }

    // Move forward in history.
    inHistory--;
    return true;
}

void PathContainer::truncateHistory()
{
    // Clean up all "redo" options:
    // Delete the last <inHistory> history entries.
    while (inHistory > 0)
    {
        // Take the last step from history (removes it from history).
        DrawHistoryStep *step = history.takeLast();
        // Delete all future objects in this step. These objects are not visible.
        step->deleteFuture();
        // Delete the step. The past objects of the step are untouched, since
        // they are still owned by other history steps or by this.
        delete step;
        --inHistory;
    }
}

void PathContainer::clearHistory(int n)
{
    // Negative values of n don't make any sense and are interpreted as 0.
    if (n < 0)
        n = 0;

    // Delete the first entries in history until
    // history.length() - inHistory <= n .
    for (int i = history.length() - inHistory; i>n; i--)
    {
        // Take the first step from history (removes it from history).
        DrawHistoryStep *step = history.takeFirst();
        // Delete all past objects in this step. These objects are not visible.
        // TODO: Does this delete the correct part?
        step->deletePast();
        // Delete the step. The future objects of the step are untouched, since
        // they are still owned by other history steps or by this.
        delete step;
    }
}

void PathContainer::clearPaths(QGraphicsScene *scene)
{
    // Create a new history step.
    DrawHistoryStep *step = new DrawHistoryStep();
    // Append all paths to the history step.
    // If scene != nullptr, additionally remove the items from the scene.
    if (scene == nullptr)
    {
        for (int i=0; i<paths.length(); ++i)
            step->addRemoveItem(i, paths[i]);
    }
    else
    {
        for (int i=0; i<paths.length(); ++i)
        {
            step->addRemoveItem(i, paths[i]);
            scene->removeItem(paths[i]);
        }
    }
    // Add the scene to history.
    history.append(step);
    // All paths have been added to history. paths can be cleared.
    paths.clear();
}
