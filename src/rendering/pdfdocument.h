// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <algorithm>
#include <memory>
#include <utility>
#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QRectF>
#include <QVector>
#include "src/config.h"
#include "src/enumerates.h"
#include "src/media/mediaannotation.h"

class AbstractRenderer;


/// Unified type of PDF links for all PDF engines.
struct PdfLink {
    /// Types of links in PDF.
    /// These are all negative, because positive values are interpreted as page
    /// numbers for internal navigation links.
    enum LinkType
    {
        /// Link of unknown type.
        NoLink = 0,
        /// Link inside the PDF document.
        PageLink,
        /// Link contains an action.
        ActionLink,
        /// Link to target in external PDF.
        ExternalPDF,
        /// Link to a remote destination.
        RemoteUrl,
        /// Link to a local file.
        LocalUrl,
        /// Link to movie annotation
        MovieLink,
        /// Link to sound annotation
        SoundLink,
    };

    /// Type of this link object
    LinkType type = NoLink;
    /// Link area on slide
    QRectF area;
    PdfLink(const LinkType type, const QRectF &area) : type(type), area(area) {}
    virtual ~PdfLink() = default;
};

/// Link to external URL
struct ExternalLink : PdfLink {
    QUrl url;
    ExternalLink(const LinkType type, const QRectF &area, const QUrl &url) : PdfLink(type, area), url(url) {}
};

/// Link to page
struct GotoLink : PdfLink {
    int page;
    GotoLink(const QRectF &area, const int page) : PdfLink(PageLink, area), page(page) {}
};

/// Link triggering an action
struct ActionLink : PdfLink {
    Action action;
    ActionLink(const QRectF &area, const Action action) : PdfLink(LinkType::ActionLink, area), action(action) {}
};

/// Link to media annotation
struct MediaLink : PdfLink {
    const std::shared_ptr<MediaAnnotation> annotation;
    MediaLink(const LinkType type, const QRectF &area, const std::shared_ptr<MediaAnnotation> annotation) :
        PdfLink(type, area), annotation(annotation) {}
};

/// PDF outline (table of contents, TOC) entry for storing a tree in a list.
struct PdfOutlineEntry {
    /// Title of the entry.
    QString title;
    /// Page index in the PDF (start counting from 0, destination resolved by the PDF engine)
    int page;
    /// Index of next outline on the same level in some data structure.
    int next;
};

/// Unified type of slide transition for all PDF engines.
struct SlideTransition {
    /// Slide tansition types as define by the PDF standard.
    /// The numbers used here are the same as in fz_transition::type and
    /// in Poppler::PageTransition::Type.
    enum Type
    {
        /// Invalid slide transition.
        Invalid = -1,
        /// No transition.
        Replace = 0,
        /// 2 lines sweep accross the screen and reveal the next page.
        Split = 1,
        /// Multiple lines seep accross the screen and reveal the next page.
        Blinds = 2,
        /// A box seeps inward or outward and reveals the next page.
        Box = 3,
        /// Single line seeping accross the screen to reveal the next page.
        Wipe = 4,
        /// Current page becomes transparent to reveal next page.
        Dissolve = 5,
        /// The screen is divided in small squares which change to the next page in pseudo-random order.
        Glitter = 6,
        /// Changes fly in or out.
        Fly = 7,
        /// Current page is pushed away by next page.
        Push = 8,
        /// Next page flies in and covers current page.
        Cover = 9,
        /// Current page flies out to uncover next page.
        Uncover = 10,
        /// Current slide dissolves, background color becomes visible, and next slide appears.
        Fade = 11,
        /// Fly animation in which a rectangle including all changes flies in.
        /// Currently not implemented and treated like Fly.
        FlyRectangle = 12,
    };

    /// Direction controlled by 2 bits for outwards and vertical.
    enum Properties
    {
        /// direction flag in to out.
        Outwards = 1 << 0,
        /// orientation flag vertical.
        Vertical = 1 << 1,
    };

    /// Type of the slide transition.
    qint8 type = Replace;

    /// Direction of the transition.
    /// first bit: inward (0) or outward (1) direction.
    /// second bit: horizontal (0) or vertical (1) direction.
    qint8 properties = 0;

    /// Angle in degrees of the direction of the direction.
    qint16 angle = 0;

    /// Transition duration in s.
    float duration = 0.;

    /// Only relevant for Fly and FlyRectangle, in [0,1].
    /// Starting point for "flying" relative to the usual "fly" path.
    float scale = 1.;

    /// Create time-reverse of slide transition (in place)
    void invert() noexcept
    {
        properties ^= Outwards;
        angle = (angle + 180) % 360;
        if (type == Cover)
            type = Uncover;
        else if (type == Uncover)
            type = Cover;
    }
};


/// Compare outline entries by their page.
inline bool operator<(const int page, const PdfOutlineEntry& other)
{
    return page < other.page;
}


/**
 * @brief Abstract class for handling PDF documents.
 *
 * This class is implemented for different PDF engines:
 * MuPDF, Poppler, and Qt PDF.
 *
 * @see MuPdfDocument
 * @see PopplerDocument
 * @see QtDocument
 */
class PdfDocument
{
protected:
    /// Modification time of the PDF file.
    QDateTime lastModified;

    /// Path to the PDF file.
    QString path;

    /// 0 if all pages have equal size, 1 otherwise, -1 if unknown.
    qint8 flexible_page_sizes = -1;

    /**
     * @brief list representing the outline tree
     *
     * The outline tree is stored as a list of PdfOutlineEntries. Each entry
     * has a property "next" which contains the index of the next entry on the
     * same outline level or minus the index of the next entry on a higher
     * level if no further entries exist on the same level. The first child of
     * the entry (if it has any children) follows immediately after the entry
     * in the list.
     *
     * The first entry is always a dummy entry with page -1 and empty title.
     * This list may never be empty or some functions will cause segmentation
     * faults.
     */
    QVector<PdfOutlineEntry> outline = {{"", -1, 1}};

public:
    /// Constructor: only initialize filename.
    explicit PdfDocument(const QString &filename) : path(filename) {}

    /// Trivial destructor.
    virtual ~PdfDocument() {}

    /// Load or reload the PDF document if the file has been modified since
    /// it was loaded. Return true if the document was reloaded.
    virtual bool loadDocument() = 0;

    /// Size of page in points (point = inch/72).
    virtual const QSizeF pageSize(const int page) const = 0;

    /// Number of pages in PDF file.
    virtual int numberOfPages() const = 0;

    /// Pdf engine.
    virtual PdfEngine type() const noexcept = 0;

    /// Create a new renderer for this document.
    virtual AbstractRenderer *createRenderer(const PagePart part = FullPage) const = 0;

    /// Label of page with given index.
    /// The default implementation returns a string representing the page number.
    virtual const QString pageLabel(const int page) const
    {return QString::number(page+1);}

    /// Label of page with given index.
    virtual int pageIndex(const QString &label) const
    {return label.toInt() - 1;}

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    virtual int overlaysShifted(const int start, const int shift_overlay) const
    {return start + (shift_overlay >= 0 ? shift_overlay & ~AnyOverlay : shift_overlay | AnyOverlay);}

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    /// The default implementation always returns an empty list.
    virtual QList<int> overlayIndices() const noexcept
    {return QList<int>();}

    /// Check whether a file has been loaded successfully.
    virtual bool isValid() const = 0;

    /// Load the PDF labels and outline, fill PdfDocument::outline.
    virtual void loadLabels() {};

    /// Search which page contains needle.
    virtual std::pair<int,QRectF> search(const QString &needle, int start_page = 0, bool forward = true) const
    {const auto [page, list] = searchAll(needle, start_page, forward); return {page, list.isEmpty() ? QRectF() : list.first()};}

    /// Search which page contains needle and return the
    /// outline of all occurrences on that slide.
    virtual std::pair<int,QList<QRectF>> searchAll(const QString &needle, int start_page = 0, bool forward = true) const
    {return {-1,{}};}

    /// get function for outline
    const QVector<PdfOutlineEntry> &getOutline() const noexcept
    {return outline;}

    /// Return outline entry at given page.
    const PdfOutlineEntry &outlineEntryAt(const int page) const
    {
        // Upper bound will always point to the next outline entry
        // (or outline.cend() or outline.cbegin()).
        const auto it = std::upper_bound(outline.cbegin(), outline.cend(), page);
        return it == outline.cbegin() ? *it : *(it-1);
    }

    /// Link at given position (in point = inch/72)
    virtual const PdfLink *linkAt(const int page, const QPointF &position) const
    {return nullptr;}

    /// List all video annotations on given page.
    virtual QList<std::shared_ptr<MediaAnnotation>> annotations(const int page)
    {return {};}

    /// Path to PDF file.
    const QString &getPath() const
    {return path;}

    /// Slide transition when reaching the given page.
    virtual const SlideTransition transition(const int page) const
    {return SlideTransition();}

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept = 0;

    /// Duration of given page in secons. Default value is -1 is interpreted as infinity.
    virtual qreal duration(const int page) const noexcept
    {return -1.;}
};

#endif // PDFDOCUMENT_H
