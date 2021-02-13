#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <QDebug>
#include <QObject>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QList>
#include "src/drawing/abstractgraphicspath.h"
#include "src/drawing/basicgraphicspath.h"
#include "src/drawing/fullgraphicspath.h"
#include "src/preferences.h"


/// One single step in the history of drawing.
/// Save deleted and added GraphicsItems and their stacking position
/// (by their index).
/// A history step consists of
/// 1. deleting QGraphicsItems. DrawHistoryStep saves these items together
///    with their index in the stacking order before they were deleted.
/// 2. creating QGraphicsItems. DrawHistoryStep saves these items together
///    with their index after all new QGraphicsItems were added.
struct DrawHistoryStep {
    friend class PathContainer;

private:
    /// Newly created items with their index after the history step.
    QMap<int, QGraphicsItem*> createdItems;

    /// Deleted items with their indices before the history step.
    QMap<int, QGraphicsItem*> deletedItems;
};


/// Collection of QGraphicsItems including a history of changes to these items.
/// This stores drawn paths for one slide even if the slide is not visible.
/// Access the history using undo and redo functions.
class PathContainer
{
private:
    /// List of currently visible paths in the order in which they were created
    /// TODO: don't use this when this is currently active on scene?
    QList<QGraphicsItem*> paths;

    /// List of changes forming the history of this, in the order in which they
    /// were created.
    QList<DrawHistoryStep*> history;

    /// Current position in history, measured from history.last().
    /// inHistory == 1 means "one change before the latest version".
    /// This may never become >=history.length().
    /// inHistory == -1 indicates that eraser microsteps are being applied.
    /// inHistory == -2 indicates that this entry has been created as a copy and has never been edited.
    int inHistory = 0;

    /// Remove all "redo" options.
    void truncateHistory();

public:
    /// Trivial constructor.
    explicit PathContainer() noexcept {}

    /// Destructor. Delete history and paths.
    ~PathContainer();

    /// Create a new PathContainer which is a copy of this but without any history.
    PathContainer *copy() const noexcept;

    /// Undo latest change. Return true on success and false on failure.
    bool undo(QGraphicsScene *scene = NULL);

    /// Redo latest change. Return true on success and false on failure.
    bool redo(QGraphicsScene *scene = NULL);

    /// Iterator over current paths.
    QList<QGraphicsItem*>::const_iterator cbegin() const noexcept
    {return paths.cbegin();}

    /// End of iterator over current paths.
    QList<QGraphicsItem*>::const_iterator cend() const noexcept
    {return paths.cend();}

    /// Clear history such that only <n> undo steps are possible.
    void clearHistory(int n = 0);

    /// Clear paths in a new history step.
    void clearPaths();

    /// Add a new QGraphisItem* in a new history step.
    void append(QGraphicsItem *item);

    /// Last QGraphicsItem* in history.
    //QGraphicsItem *last()
    //{return paths.last();}

    /// Start eraser step. An eraser step in history is caused by multiple
    /// events (eraser move events), which are all managed by PathContainer.
    /// We call them micro steps. Start erasing by initializing an eraser
    /// step in history.
    void startMicroStep();

    /// Apply the micro steps forming an eraser step. In the eraser micro steps
    /// paths and history.last() do not have the usual form. Instead of
    /// top-level QGraphicsItems, paths also contains QGraphicsItemGroups of
    /// paths split by erasing. This is all fixed and brought to the usual form
    /// in applyMicroStep().
    /// Return true if anything has changed.
    bool applyMicroStep();

    /// Single eraser move event. This erasees paths at pos with given eraser
    /// size. Before this function startMicroStep() has to be called and
    /// afterwards a call to applyMicroStep() is necessary.
    void eraserMicroStep(const QPointF &pos, const qreal size = 10.);

    /// Check if this contains any information.
    bool isEmpty() const noexcept
    {return paths.isEmpty() && history.isEmpty();}

    /// Check if this is an unchanged copy of another PathContainer.
    bool isPlainCopy() const noexcept
    {return inHistory == -2 && history.isEmpty();}

    void writeXml(QXmlStreamWriter &writer) const;
};

#endif // PATHCONTAINER_H
