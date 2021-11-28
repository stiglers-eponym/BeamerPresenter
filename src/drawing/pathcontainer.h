#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <QList>
#include "src/drawing/tool.h"

class QGraphicsScene;
class QGraphicsItem;
class QXmlStreamReader;
class QXmlStreamWriter;
class TextGraphicsItem;

static const QMap<Tool::BasicTool, QString> xournal_tool_names
{
    {Tool::Pen, "pen"},
    {Tool::FixedWidthPen, "pen"},
    {Tool::Highlighter, "highlighter"},
    {Tool::TextInputTool, "text"},
};


/**
 * @class PathContainer
 * @brief collection of QGraphicsItems including a history of changes to these items
 *
 * This stores drawn paths for one slide even if the slide is not visible.
 * Access the history using undo and redo functions.
 */
class PathContainer : public QObject
{
    Q_OBJECT

public:
    /**
     * One single step in the history of drawing.
     *  Save deleted and added GraphicsItems and their stacking position
     *  (by their index).
     *  A history step consists of
     *  1. deleting QGraphicsItems. DrawHistoryStep saves these items together
     *     with their index in the stacking order before they were deleted.
     *  2. creating QGraphicsItems. DrawHistoryStep saves these items together
     *     with their index after all new QGraphicsItems were added.
     */
    struct DrawHistoryStep {
        friend class PathContainer;

    private:
        /// Newly created items with their index after the history step.
        QMap<int, QGraphicsItem*> createdItems;

        /// Deleted items with their indices before the history step.
        QMap<int, QGraphicsItem*> deletedItems;
    };

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
    explicit PathContainer(QObject *parent = NULL) noexcept : QObject(parent) {}

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

    /// Check if this currently has any paths (history is ignored).
    bool isCleared() const noexcept
    {return paths.isEmpty();}

    /// Check if this is an unchanged copy of another PathContainer.
    bool isPlainCopy() const noexcept
    {return inHistory == -2;}

    /// Save drawings in xml format.
    void writeXml(QXmlStreamWriter &writer) const;

    /// Load drawings for one specific page.
    void loadDrawings(QXmlStreamReader &reader);
    /// Load drawings for one specific page in left and right page part.
    static void loadDrawings(QXmlStreamReader &reader, PathContainer *left, PathContainer *right, const qreal page_half);

    /// Bounding box of all drawings.
    QRectF boundingBox() const noexcept;

public slots:
    /// Items (currently only text items) may detect that they should be removed.
    /// They can then inform this function, which removes them and adds this as a new history step.
    void removeItem(QGraphicsItem *item);
    /// Notify of a change in a text item, add the item to history if necessary.
    void addTextItem(QGraphicsItem *item);
};

/// Convert color to string with format #RRGGBBAA
/// (required for Xournal++ format).
QString color_to_rgba(const QColor &color);

/// Convert color string of format #RRGGBBAA to QColor
/// (required for Xournal++ format).
QColor rgba_to_color(const QString &string);

#endif // PATHCONTAINER_H
