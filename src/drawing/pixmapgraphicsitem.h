// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PIXMAPGRAPHICSITEM_H
#define PIXMAPGRAPHICSITEM_H

#include <QtCore>
#include <QSet>
#include <QMap>
#include <QPixmap>
#include <QSizeF>
#include <QRectF>
#include <QGraphicsObject>
#include "src/config.h"

#define BLINDS_NUMBER_H 6
#define BLINDS_NUMBER_V 8
#define GLITTER_ROW 71
#define GLITTER_NUMBER 137

class QPainter;
class QWidget;
class QStyleOptionGraphicsItem;

/**
 * @brief pixmaps with different resolutions of same picture as QGraphicsItem
 *
 * Store multiple pixmaps for the same vector graphic at different resolutions.
 * This makes it possible to use the page background as a QGraphicsItem
 * while showing different pixmaps with the correct resolution for different
 * views of the QGraphicsScene.
 */
class PixmapGraphicsItem : public QGraphicsObject
{
    Q_OBJECT
    // Not sure what this is good for...
    Q_INTERFACES(QGraphicsItem)

    // This class is used in animations. Define properties for these animations.
    // TODO: bundle these animation properties in a std::variant?
    /// Opacity of this QGraphicsItem.
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    /// Position/shift in x direction.
    Q_PROPERTY(qreal x READ x WRITE setX)
    /// Position/shift in y direction.
    Q_PROPERTY(qreal y READ y WRITE setY)
    /// mask for rendering this
    Q_PROPERTY(QRectF mask READ mask WRITE setMask)
    /// progress of glitter animation.
    Q_PROPERTY(int progress READ progress WRITE setProgress)

public:
    /// Different masks are used in animations. MaskType defines how
    /// this is drawn and how _mask is interpreted.
    enum MaskType
    {
        NoMask,
        PositiveClipping,
        NegativeClipping,
        HorizontalBlinds,
        VerticalBlinds,
        Glitter,
    };

private:
    /// map pixmap width (in pixels) to pixmaps
    QMap<unsigned int, QPixmap> pixmaps;

    /// Bouding rect of this QGraphicsItem.
    QRectF bounding_rect;

    /// Rectangular mask. Depending on mask_type, painting will be clipped to mask,
    /// to the inverse of mask, or to repeated copies of mask.
    QRectF _mask;

    /// Animation progress, only relevant for glitter transition.
    /// A value of UINT_MAX means that no glitter animation is currently active.
    unsigned int animation_progress = UINT_MAX;

    /// Mask type defines meaning of mask and depends on the slide transition.
    MaskType mask_type = NoMask;

    /// List of hashs (widths) of pixmaps which were added since the latest
    /// call to trackChanges().
    QSet<unsigned int> newHashs;

public:
    /// Type of this custom QGraphicsItem.
    enum {Type = UserType + 4};

    /// Trivial constructor.
    explicit PixmapGraphicsItem(const QRectF &rect, QGraphicsItem *parent = NULL) :
        QGraphicsObject(parent), bounding_rect(rect) {}

    /// @return custom QGraphicsItem type
    int type() const noexcept override
    {return Type;}

    /// Check whether this contains a pixmap with the given width or with width+1.
    bool hasWidth(const unsigned int width) const noexcept;

    /// Paint this on given painter.
    /// @param painter paint to this painter.
    /// @param option currently ignored.
    /// @param widget currently ignored.
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = NULL) override;

    /// Bounding rect in scene coordinates
    QRectF boundingRect() const override
    {return bounding_rect;}

    /// Get pixmap with the given width or next larger width.
    QPixmap getPixmap(const unsigned int width) const noexcept;

    /// @return _mask
    /// @see setMask()
    const QRectF mask() const noexcept
    {return _mask;}

    /// @return mask_type
    /// @see setMaskType()
    MaskType maskType() const noexcept
    {return mask_type;}

    /// @return animation_progress (used in glitter animation).
    /// @see setProgress()
    int progress() const noexcept
    {return animation_progress;}

    /// @return number of pixmaps.
    int number() const noexcept
    {return pixmaps.size();}

#ifdef QT_DEBUG
    /// Only for debugging.
    /// @return list all available widths
    QList<unsigned int> widths() const
    {return pixmaps.keys();}
#endif

public slots:
    /// Add a pixmap.
    void addPixmap(const QPixmap& pixmap) noexcept;

    /// Set (overwrite) bounding rect.
    void setRect(const QRectF &rect) noexcept
    {bounding_rect = rect; update();}

    /// Set (overwrite) bounding rect size.
    void setSize(const QSizeF &size) noexcept
    {bounding_rect.setSize(size);}

    /// Clear everything.
    void clearPixmaps() noexcept
    {pixmaps.clear();}

    /// Start tracking changes.
    /// @see clearOld()
    void trackNew() noexcept
    {newHashs.clear();}

    /// Clear everything that was added or modified before the latest call to trackNew().
    /// @see trackNew()
    void clearOld() noexcept;

    /// Set rectangular mask.
    /// @see mask()
    void setMask(const QRectF &rect) noexcept
    {_mask = rect; update();}

    /// Set mask type (meaning of mask).
    /// @see maskType()
    void setMaskType(const MaskType type) noexcept;

    /// Set animation progress (only in glitter transition).
    /// @see progress()
    void setProgress(const int progress) noexcept
    {animation_progress = progress; update();}
};

#endif // PIXMAPGRAPHICSITEM_H
