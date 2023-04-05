// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <QPointF>
#include <QTransform>
#include <QString>
#include <QLatin1Char>
#include <QList>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QDataStream>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QBrush>
#include "src/config.h"
#include "src/drawing/drawtool.h"

class QGraphicsItem;
class QGraphicsScene;
class QXmlStreamReader;
class QXmlStreamWriter;
class TextGraphicsItem;

namespace drawHistory
{
    /// Store changes in a draw tool used to stroke a path.
    struct DrawToolDifference {
        QPen old_pen; ///< pen of the old tool
        QPen new_pen; ///< pen of the new tool
        QBrush old_brush; ///< brush of the old tool
        QBrush new_brush; ///< brush of the new tool
        QPainter::CompositionMode old_mode; ///< composition mode of the old tool
        QPainter::CompositionMode new_mode; ///< composition mode of the new tool
        DrawToolDifference(const DrawTool &old_tool, const DrawTool &new_tool) :
            old_pen(old_tool.pen()), new_pen(new_tool.pen()), old_brush(old_tool.brush()), new_brush(new_tool.brush()), old_mode(old_tool.compositionMode()), new_mode(new_tool.compositionMode()) {}
    };
    /// Store changes in text properties (not text content!).
    struct TextPropertiesDifference {
        QFont old_font; ///< old font
        QFont new_font; ///< new font
        QRgb color_diff; ///< binary difference of colors in RGBA (4 byte) format
    };

    /**
     * One single step in the history of drawing.
     *  Save deleted and added GraphicsItems and their stacking position
     *  (by their index).
     *  A history step consists of
     *  1. deleting QGraphicsItems. drawHistory::Step saves these items together
     *     with their index in the stacking order before they were deleted.
     *  2. creating QGraphicsItems. drawHistory::Step saves these items together
     *     with their index after all new QGraphicsItems were added.
     *  3. changing tools of items. For paths and text fields the tool or
     *     properties are changed.
     *  4. transforming items. drawHistory::Step saves the transformations
     *     together with their index after all new QGraphicsItems were added.
     */
    struct Step {
        /// Items with the transformation applied in this history step.
        QHash<QGraphicsItem*, QTransform> transformedItems;

        /// Changes of draw tool.
        QHash<QGraphicsItem*, DrawToolDifference> drawToolChanges;

        /// Changes of text properties.
        QHash<QGraphicsItem*, TextPropertiesDifference> textPropertiesChanges;

        /// Newly created items with their index after the history step.
        QMap<int, QGraphicsItem*> createdItems;

        /// Deleted items with their indices before the history step.
        QMap<int, QGraphicsItem*> deletedItems;

        /// Check whether this step includes any changes.
        bool isEmpty() const {
            return transformedItems.isEmpty()
                    && drawToolChanges.isEmpty()
                    && textPropertiesChanges.isEmpty()
                    && createdItems.isEmpty()
                    && deletedItems.isEmpty();
        }
    };
}
Q_DECLARE_METATYPE(drawHistory::Step);


/**
 * @brief Collection of QGraphicsItems including a history of changes to these items
 *
 * This stores drawn paths for one slide even if the slide is not visible.
 * Access the history using undo and redo functions.
 */
class PathContainer : public QObject
{
    Q_OBJECT

private:
    /// List of currently visible paths in the order in which they were created
    QList<QGraphicsItem*> paths;

    /// List of changes forming the history of this, in the order in which they
    /// were created.
    QList<drawHistory::Step*> history;

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

    /// Append drawHistory::Step.
    void addHistoryStep(drawHistory::Step *step);

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
inline QString color_to_rgba(const QColor &color) noexcept
{
    return QLatin1Char('#') + QString::number((color.rgb() << 8) + color.alpha(), 16).rightJustified(8, '0', true);
}

/// Convert color string of format #RRGGBBAA to QColor
/// (required for Xournal++ format).
inline QColor rgba_to_color(const QString &string) noexcept
{
    if (string.length() == 9)
        return QColor('#' + string.right(2) + string.mid(1,6));
    return QColor(string);
}

QDataStream &operator<<(QDataStream &stream, const QGraphicsItem *item);
QDataStream &operator>>(QDataStream &stream, QGraphicsItem *&item);

#endif // PATHCONTAINER_H
