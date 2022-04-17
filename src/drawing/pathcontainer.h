#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <QList>
#include <QGraphicsItem>
#include <QDataStream>
#include "src/drawing/tool.h"

class QGraphicsScene;
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
 * @brief Collection of QGraphicsItems including a history of changes to these items
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
     *  3. transforming items. DrawHistoryStep saves the transformations
     *     together with their index after all new QGraphicsItems were added.
     */
    struct DrawHistoryStep {
        friend class PathContainer;

    private:
        /// Items with the transformation applied in this history step
        /// and with their indices after the history step.
        QMap<int, QTransform> transformedItems;

        /// Newly created items with their index after the history step.
        QMap<int, QGraphicsItem*> createdItems;

        /// Deleted items with their indices before the history step.
        QMap<int, QGraphicsItem*> deletedItems;
    };

private:
    /// List of currently visible paths in the order in which they were created
    QList<QGraphicsItem*> paths;

    /// List of changes forming the history of this, in the order in which they
    /// were created.
    QList<DrawHistoryStep*> history;

    /**
     * Current position in history, measured from history.last().
     *
     * inHistory == 1 means "one change before the latest version".
     * This may never become >=history.length().
     *
     * Special values are:
     * * inHistory == -1 indicates that eraser microsteps are being applied.
     * * inHistory == -2 indicates that this entry has been created as a copy and has never been edited.
     */
    int inHistory = 0;

    /// Remove all "redo" options.
    void truncateHistory();

public:
    /// Trivial constructor.
    explicit PathContainer(QObject *parent = NULL) noexcept : QObject(parent) {}

    /// Destructor. Delete history and paths.
    ~PathContainer();

    /// Create a new PathContainer which is a copy of this but does not have any history.
    PathContainer *copy() const noexcept;

    /// Undo latest change.
    /// @return true on success and false on failure.
    /// @see redo()
    bool undo(QGraphicsScene *scene = NULL);

    /// Redo latest change.
    /// @return true on success and false on failure.
    /// @see undo()
    bool redo(QGraphicsScene *scene = NULL);

    /// Iterator over current paths.
    /// @see cend()
    QList<QGraphicsItem*>::const_iterator cbegin() const noexcept
    {return paths.cbegin();}

    /// End of iterator over current paths.
    /// @see cbegin()
    QList<QGraphicsItem*>::const_iterator cend() const noexcept
    {return paths.cend();}

    /// Clear history such that only n undo steps are possible.
    void clearHistory(int n = 0);

    /// Clear paths in a new history step.
    void clearPaths();

    /// Add a new QGraphisItem* in a new history step.
    void append(QGraphicsItem *item);

    /**
     * Start eraser step. An eraser step in history is caused by multiple
     * events (eraser move events), which are all managed by PathContainer.
     * We call them micro steps. Start erasing by initializing an eraser
     * step in history.
     * @see eraserMicroStep()
     * @see applyMicroStep()
     */
    void startMicroStep();

    /**
     * Apply the micro steps forming an eraser step. In the eraser micro steps
     * paths and history.last() do not have the usual form. Instead of
     * top-level QGraphicsItems, paths also contains QGraphicsItemGroups of
     * paths split by erasing. This is all fixed and brought to the usual form
     * in applyMicroStep().
     * @return true if anything has changed, false otherwise.
     * @see startMicroStep()
     * @see eraserMicroStep()
     */
    bool applyMicroStep();

    /**
     * Single eraser move event. This erasees paths at scene_pos with given
     * eraser size. Before this function startMicroStep() has to be called and
     * afterwards a call to applyMicroStep() is necessary.
     * @see startMicroStep()
     * @see applyMicroStep()
     */
    void eraserMicroStep(const QPointF &scene_pos, const qreal size = 10.);

    /// Check if this contains any information.
    /// @return true if this contains any elements or history steps.
    bool isEmpty() const noexcept
    {return paths.isEmpty() && history.isEmpty();}

    /// Check if this currently has any paths (history is ignored).
    bool isCleared() const noexcept
    {return paths.isEmpty();}

    /// Check if this is an unchanged copy of another PathContainer.
    /// @return true if inHistory == -2
    bool isPlainCopy() const noexcept
    {return inHistory == -2;}

    /// Save drawings in xml format.
    /// @see loadDrawings(QXmlStreamReader &reader)
    void writeXml(QXmlStreamWriter &writer) const;

    /// Load drawings for one specific page.
    /// @see writeXml(QXmlStreamWriter &writer) const
    void loadDrawings(QXmlStreamReader &reader);

    /// Load drawings for one specific page in left and right page part.
    /// @see loadDrawings(QXmlStreamReader &reader)
    /// @see writeXml(QXmlStreamWriter &writer) const
    static void loadDrawings(QXmlStreamReader &reader, PathContainer *left, PathContainer *right, const qreal page_half);

    /// @return bounding box of all drawings
    QRectF boundingBox() const noexcept;

    /// Create history step that replaces the old item by the new one.
    /// If the new item is NULL, the old item is deleted.
    /// If the old item is NULL, the new one is just inserted.
    /// Items (currently only text items) may detect that they should be removed.
    /// They can then inform this function, which removes them and adds this as a new history step.
    void replaceItem(QGraphicsItem *olditem, QGraphicsItem *newitem);

    /// Add new paths.
    void addItems(const QList<QGraphicsItem*> &items);
    /// Remove paths.
    void removeItems(const QList<QGraphicsItem*> &items);

    // Apply transforms to items.
    void transformItemsMap(const QHash<QGraphicsItem*, QTransform> &map);

public slots:
    // Remove the item in a new history step.
    void removeItem(QGraphicsItem *item)
    {replaceItem(item, NULL);}

    /// Notify of a change in a text item, add the item to history if necessary.
    void addTextItem(QGraphicsItem *item)
    {replaceItem(NULL, item);}
};

/// Convert color to string with format #RRGGBBAA
/// (required for Xournal++ format).
QString color_to_rgba(const QColor &color);

/// Convert color string of format #RRGGBBAA to QColor
/// (required for Xournal++ format).
QColor rgba_to_color(const QString &string);

QDataStream &operator<<(QDataStream &stream, const QGraphicsItem &item);
QDataStream &operator>>(QDataStream &stream, QGraphicsItem &item);

#endif // PATHCONTAINER_H
