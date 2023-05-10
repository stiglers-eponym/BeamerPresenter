// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PATHCONTAINER_H
#define PATHCONTAINER_H

#include <set>
#include <map>
#include <unordered_map>
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
#include <QGraphicsItem>
#include "src/config.h"
#include "src/preferences.h"
#include "src/drawing/drawtool.h"

class QGraphicsScene;
class AbstractGraphicsPath;
class TextGraphicsItem;
class QXmlStreamReader;
class QXmlStreamWriter;

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
            old_pen(old_tool.pen()), new_pen(new_tool.pen()),
            old_brush(old_tool.brush()), new_brush(new_tool.brush()),
            old_mode(old_tool.compositionMode()), new_mode(new_tool.compositionMode()) {}
    };
    /// Store changes in text properties (not text content!).
    struct TextPropertiesDifference {
        QFont old_font; ///< old font
        QFont new_font; ///< new font
        QRgb color_diff; ///< binary difference of colors in RGBA (4 byte) format
    };
    /// Change in the Z value (of a QGraphicsItem)
    struct ZValueChange {
        qreal old_z; ///< old Z value
        qreal new_z;///< new Z value
    };

    /**
     * One single step in the history of drawing.
     * This uses std::map to enable ranged expressions also in Qt 5.
     */
    struct Step {
        // TODO: check if a std::variant would provide a more efficient history:
        //std::map<QGraphicsItem*, std::variant<ZValueChange,QTransform,DrawToolDifference,TextPropertiesDifference>> changes;

        /// Changes in the order of items.
        std::map<QGraphicsItem*, ZValueChange> z_value_changes;

        /// Items with the transformation applied in this history step.
        std::map<QGraphicsItem*, QTransform> transformedItems;

        /// Changes of draw tool.
        std::map<AbstractGraphicsPath*, DrawToolDifference> drawToolChanges;

        /// Changes of text properties.
        std::map<TextGraphicsItem*, TextPropertiesDifference> textPropertiesChanges;

        /// Newly created items with their index after the history step.
        QList<QGraphicsItem*> createdItems;

        /// Deleted items with their indices before the history step.
        QList<QGraphicsItem*> deletedItems;

        /// Check whether this step includes any changes.
        bool empty() const {
            return transformedItems.empty()
                    && drawToolChanges.empty()
                    && textPropertiesChanges.empty()
                    && createdItems.empty()
                    && deletedItems.empty()
                    && z_value_changes.empty();
        }
    };
}
Q_DECLARE_METATYPE(drawHistory::Step);


/// Compare QGraphicsItems by their z value.
inline bool cmp_by_z(QGraphicsItem *left, QGraphicsItem *right) noexcept
{
    return left && right && left->zValue() < right->zValue();
}

/**
 * @brief Collection of QGraphicsItems including a history of changes to these items
 *
 * This stores drawn paths for one slide even if the slide is not visible.
 * Access the history using undo and redo functions.
 *
 * Memory management
 * From SlideScene we often get QGraphicsItem* pointers, which we need to
 * handle. We keep some of these QGraphicsItem* objects in the history to
 * be able to undo, e.g., deleting an item. To avoid leaking memory, we
 * keep a reference count of the items in _ref_count. There we store for
 * each object the number of references (pointers) to the object which
 * are managed by this. When _ref_count reaches 0, the object is deleted
 * except if it is currently visible.
 *
 * Stacking order
 * The stacking order of items is defined by their z value. To be able to
 * insert items anywhere in the stacking order, we keep a set of all
 * items sorted by their z value in _z_order. When an item is deleted
 * from _ref_count, it must also be removed from _z_order.
 *
 * It is important that the z order of items managed by PathContainer
 * remains constant!
 */
class PathContainer : public QObject
{
    Q_OBJECT

private:
    /**
     * Entries in the lookup table of all items.
     * ref_count counts references to the given QGraphicsItem*.
     * If ref_count reaches 0, the item is deleted except if it is marked
     * visibe. Visible items are deleted when ref_count reaches -1.
     */
    struct LookUpProperties {
        /// Number of references to a QGraphicsItem*.
        int ref_count = 0;
        /// Visibility of the item if the correct page were shown.
        /// Visible items are only deleted when ref_count reaches -1.
        bool visible = false;
    };
    /// Lookup table of items managed by this container.
    /// This includes all items in the history.
    std::unordered_map<QGraphicsItem*,LookUpProperties> _ref_count;

    /// Set of all items for efficient lookup of items by z value.
    /// This set owns nothing and is sorted by z_value.
    /// It contains all items, including history.
    std::multiset<QGraphicsItem*, decltype(&cmp_by_z)> _z_order{&cmp_by_z};

    /// List of changes forming the history of this, in the order in which they
    /// were created.
    QList<drawHistory::Step> history;

    /**
     * Decrease reference count for item. Delete item if reference
     * count reaches zero. Only deletes item if item was in _ref_count. */
    void releaseItem(QGraphicsItem *item) noexcept;
    /// Increase reference count for item
    void keepItem(QGraphicsItem *item) noexcept
    {++(_ref_count[item].ref_count);}
    /**
     * Increase reference count for item and mark item as visible/hidden.
     * This does not show or hide the item, but only marks it as
     * visible/hidden in a lookup table. */
    void keepItem(QGraphicsItem *item, const bool visible) noexcept
    {
        LookUpProperties &prop = _ref_count[item];
        ++prop.ref_count;
        prop.visible = visible;
    }
    /// Cleans up items in a history step.
    void deleteStep(const drawHistory::Step &step) noexcept;

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
    /// Limit history to default length.
    void limitHistory()
    {
        const int target_length = preferences()->history_length_visible_slides;
        if (history.length() > target_length)
            clearHistory(target_length);
    }

    /// Remove a given item from _z_order. If the item is not found in the
    /// ordered array, the full array is sorted, assuming that z values have
    /// changed.
    void removeFromZOrder(QGraphicsItem *item) noexcept;

public:
    /**
     * Iterator over all items managed by this which are marked visible.
     * Items in history which should not be visible are skipped.
     */
    struct VisibleIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = QHash<QGraphicsItem*,LookUpProperties>::difference_type;
        using value_type        = QGraphicsItem*;
        using pointer           = QGraphicsItem* const*;
        using reference         = QGraphicsItem* const&;
        VisibleIterator(
            const std::unordered_map<QGraphicsItem*,LookUpProperties>::const_iterator &it,
            const std::unordered_map<QGraphicsItem*,LookUpProperties> &map) :
            _it(it), _map(map) {}
        reference operator*() const {return _it->first;}
        pointer operator->() {return &(_it->first);}
        VisibleIterator& operator++()
            {while (++_it != _map.cend() && !_it->second.visible) {}; return *this;}
        VisibleIterator operator++(int)
            {VisibleIterator tmp = *this; ++(*this); return tmp;}
        friend bool operator== (const VisibleIterator& a, const VisibleIterator& b)
            {return a._it == b._it;};
        friend bool operator!= (const VisibleIterator& a, const VisibleIterator& b)
            {return a._it != b._it;};
    private:
        /// underlying iterator, of which hidden elements are skipped.
        std::unordered_map<QGraphicsItem*,LookUpProperties>::const_iterator _it;
        /// QHash on which this iterator is used.
        const std::unordered_map<QGraphicsItem*,LookUpProperties> &_map;
    };

    /// Const iterator over visible items.
    VisibleIterator begin() const
    {
        auto it = _ref_count.cbegin();
        if (_ref_count.empty())
            return VisibleIterator(it, _ref_count);
        const auto &end = _ref_count.cend();
        while (!it->second.visible && ++it != end) {}
        return VisibleIterator(it, _ref_count);
    }
    /// Const iterator over visible items.
    VisibleIterator end() const
    {return VisibleIterator(_ref_count.cend(), _ref_count);}

    /// Trivial constructor.
    explicit PathContainer(QObject *parent = nullptr) noexcept : QObject(parent) {}

    /// Destructor. Delete history and paths.
    ~PathContainer();

    /// Highest currently used z value
    qreal topZValue() const noexcept
    {if (_z_order.empty()) return 10; return (*_z_order.crbegin())->zValue();}

    /// Get z value for stacking after item.
    qreal zValueAfter(const QGraphicsItem *item) const noexcept;

    /// Lowest currently used z value
    qreal bottomZValue() const noexcept
    {if (_z_order.empty()) return 0; return (*_z_order.cbegin())->zValue();}

    /// Create a new PathContainer which is a copy of this but does not have any history.
    PathContainer *copy() const noexcept;

    /// Undo latest change.
    /// @return true on success and false on failure.
    /// @see redo()
    bool undo(QGraphicsScene *scene = nullptr);

    /// Redo latest change.
    /// @return true on success and false on failure.
    /// @see undo()
    bool redo(QGraphicsScene *scene = nullptr);

    /// Clear history such that only n undo steps are possible.
    void clearHistory(int n = 0);

    /// Clear paths in a new history step.
    bool clearPaths();

    /// Add a new QGraphisItem* in a new history step and bring it to the foreground.
    void appendForeground(QGraphicsItem *item);

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
    bool empty() const noexcept
    {return _ref_count.empty();}

    /// Check if this currently has any paths (history is ignored).
    bool isCleared() const noexcept;

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
    void addItemsForeground(const QList<QGraphicsItem*> &items);
    /// Remove paths.
    void removeItems(const QList<QGraphicsItem*> &items);

    /// Bring list of items to foreground and add history step.
    bool bringToForeground(const QList<QGraphicsItem*> &to_foreground);

    /// Bring list of items to background and add history step.
    bool bringToBackground(const QList<QGraphicsItem*> &to_background);

    /// Create history step with given changes. Pointers may be
    /// nullptr. This assumes that the items are already owned by this.
    bool addChanges(std::map<QGraphicsItem*, QTransform> *transforms,
                    std::map<AbstractGraphicsPath*, drawHistory::DrawToolDifference> *tools,
                    std::map<TextGraphicsItem*, drawHistory::TextPropertiesDifference> *texts);

public slots:
    // Remove the item in a new history step.
    void removeItem(QGraphicsItem *item)
    {replaceItem(item, nullptr);}

    /// Notify of a change in a text item, add the item to history if necessary.
    void addTextItem(QGraphicsItem *item)
    {replaceItem(nullptr, item);}
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

/// Add item to stream. Currently this only supports
/// BasicGraphicsPath, FullGraphicsPath, and TextGraphicsItem.
QDataStream &operator<<(QDataStream &stream, const QGraphicsItem *item);
/// Read item from stream. Currently this only supports
/// BasicGraphicsPath, FullGraphicsPath, and TextGraphicsItem.
QDataStream &operator>>(QDataStream &stream, QGraphicsItem *&item);

#endif // PATHCONTAINER_H
