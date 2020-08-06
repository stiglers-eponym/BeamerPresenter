#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <QDebug>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QList>
#include "src/drawing/drawhistorystep.h"
#include "src/drawing/abstractgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/fullgraphicspath.h"

/// Collection of QGraphicsItems including a history of changes to these items.
/// This stores drawn paths per slide even if the slide is not visible.
/// Access the history using undo and redo functions.
class PathContainer : public QObject
{
    Q_OBJECT

private:
    /// List of currently visible paths in the order in which they were created
    /// TODO: don't use this when this is currently active on scene?
    QList<QGraphicsItem*> paths;

    /// List of changes forming the history of this, in the order in which they
    /// were created.
    QList<DrawHistoryStep*> history;

    /// Current position in history, measured from history.last().
    /// inHistory == 1 means "one change before the latest version".
    /// This may never become negative or >=history.length().
    int inHistory = 0;

    /// Remove all "redo" options.
    void truncateHistory();

public:
    explicit PathContainer(QObject *parent = nullptr);
    ~PathContainer();

    /// Undo latest change. Return true on success and false on failure.
    bool undo(QGraphicsScene *scene = nullptr);

    /// Redo latest change. Return true on success and false on failure.
    bool redo(QGraphicsScene *scene = nullptr);

    QList<QGraphicsItem*>::const_iterator cbegin() const
    {return paths.cbegin();}

    QList<QGraphicsItem*>::const_iterator cend() const
    {return paths.cend();}

    /// Clear history such that only <n> undo steps are possible.
    void clearHistory(int n = 0);

    /// Move all paths to history.
    void clearPaths();

    void append(QGraphicsItem *item);

    QGraphicsItem *last()
    {return paths.last();}

    void startMicroStep();
    void applyMicroStep();
    void eraserMicroStep(const QPointF &pos);

signals:

};

#endif // PATHCONTAINER_H
