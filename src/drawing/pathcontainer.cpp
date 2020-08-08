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
    inHistory++;

    // First remove newly created items.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &removeItems = history[history.length()-inHistory]->getCreatedItems();
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = removeItems.constEnd(); it-- != removeItems.constBegin();)
    {
        paths.removeAt(it.key());
        if (it.value()->scene())
            it.value()->scene()->removeItem(it.value());
    }

    // Restore old items.
    // Get the old items from history.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->getDeletedItems();
    for (auto it = oldItems.constBegin(); it != oldItems.constEnd(); ++it)
    {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene)
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

    qDebug() << "redo";
    // First remove items which were deleted in this step.
    // Get the (sorted) indices of items which should be removed.
    const QMap<int, QGraphicsItem*> &oldItems = history[history.length()-inHistory]->getDeletedItems();
    // Iterate over the keys in reverse order, because otherwise the indices of
    // items which we still want to delete would change.
    for (auto it = oldItems.constEnd(); it-- != oldItems.constBegin();) {
        paths.removeAt(it.key());
        if (it.value()->scene())
            it.value()->scene()->removeItem(it.value());
    }

    // Restore newly created items.
    // Get the new items from history.
    const QMap<int, QGraphicsItem*> &newItems = history[history.length()-inHistory]->getCreatedItems();
    for (auto it = newItems.constBegin(); it != newItems.constEnd(); ++it) {
        paths.insert(it.key(), it.value());
        // TODO: check if it is necessary to show items explicitly.
        if (scene) {
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
    qDebug() << "Clearing" << inHistory << "history steps";
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

void PathContainer::clearPaths()
{
    truncateHistory();
    // Create a new history step.
    DrawHistoryStep *step = new DrawHistoryStep();
    // Append all paths to the history step.
    // If scene != nullptr, additionally remove the items from the scene.
    auto it = paths.cbegin();
    for (int i=0; i<paths.length(); i++, ++it)
    {
        step->addRemoveItem(i, *it);
        if ((*it)->scene())
            (*it)->scene()->removeItem(*it);
    }
    // Add the scene to history.
    history.append(step);
    // All paths have been added to history. paths can be cleared.
    paths.clear();
}

void PathContainer::append(QGraphicsItem *item)
{
    truncateHistory();
    const auto step = new DrawHistoryStep();
    step->addCreateItem(paths.length(), item);
    history.append(step);
    paths.append(item);
}

void PathContainer::startMicroStep()
{
    qDebug() << "Start micro step";
    truncateHistory();
    history.append(new DrawHistoryStep());
    inHistory = 1;
}

void PathContainer::eraserMicroStep(const QPointF &pos, const qreal size)
{
    // TODO: Works only for a single step!
    // The whole concept should be changed!
    qDebug() << "Eraser micro step";
    if (inHistory != 1)
        return;
    QList<QGraphicsItem*>::iterator path_it = paths.begin();
    for (int i=0; path_it != paths.cend(); ++path_it, i++)
    {
        // TODO: contains is not sufficient, because the eraser has a finite size.
        if (*path_it && (*path_it)->boundingRect().marginsAdded(QMargins(size, size, size, size)).contains(pos))
        {
            QGraphicsScene *scene = (*path_it)->scene();
            switch ((*path_it)->type())
            {
            case AbstractGraphicsPath::Type:
            case FullGraphicsPath::Type:
            case BasicGraphicsPath::Type:
            {
                AbstractGraphicsPath *path = static_cast<AbstractGraphicsPath*>(*path_it);
                QList<AbstractGraphicsPath*> list = path->splitErase(pos, size);
                if (list.isEmpty())
                {
                    history.last()->addRemoveItem(i, path);
                    if (scene)
                        scene->removeItem(path);
                    else
                        path->hide();
                    *path_it = nullptr;
                }
                // If nothing should change, the list will only contain a nullptr.
                else if (list.first())
                {
                    history.last()->addRemoveItem(i, path);
                    QGraphicsItemGroup *group = new QGraphicsItemGroup();
                    for (const auto item : list)
                    {
                        group->addToGroup(item);
                        item->show(); // TODO: necessary?
                    }
                    if (scene)
                    {
                        scene->addItem(group);
                        group->stackBefore(path);
                        scene->removeItem(path);
                    }
                    else
                        path->hide();
                    *path_it = group;
                }
                break;
            }
            case QGraphicsItemGroup::Type:
            {
                QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(*path_it);
                for (auto child : group->childItems())
                {
                    if (child && (child->type() == FullGraphicsPath::Type || child->type() == BasicGraphicsPath::Type))
                    {
                        const auto list = static_cast<AbstractGraphicsPath*>(child)->splitErase(pos, size);
                        if (list.isEmpty() || list.first())
                        {
                            for (auto item : list)
                                group->addToGroup(item);
                            group->removeFromGroup(child);
                            if (scene)
                                scene->removeItem(child);
                            delete child; // TODO: Check if this breaks something.
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void PathContainer::applyMicroStep()
{
    qDebug() << "Apply micro step";
    if (inHistory != 1)
        return;

    const QMap<int, QGraphicsItem*> &oldItems = history.last()->getDeletedItems();
    int shift = 0;
    for (auto it = oldItems.cbegin(); it != oldItems.cend(); ++it, shift--)
    {
        QGraphicsItem *item = paths.value(it.key());
        if (item && item->type() == QGraphicsItemGroup::Type)
        {
            QGraphicsItemGroup *group = static_cast<QGraphicsItemGroup*>(item);
            for (auto child : group->childItems())
            {
                history.last()->addCreateItem(it.key() + shift++, child);
                group->removeFromGroup(child);
                child->stackBefore(group);
            }
        }
    }
    {
    auto key = oldItems.keyEnd();
    while (key != oldItems.keyBegin())
        paths.removeAt(*--key);
    }

    qDebug() << "create";
    // Add newly created items to path.
    // Get the new items from history.
    const QMap<int, QGraphicsItem*> &newItems = history.last()->getCreatedItems();
    for (auto it = newItems.cbegin(); it != newItems.cend(); ++it)
        paths.insert(it.key(), it.value());

    // Move forward in history.
    inHistory = 0;
}
