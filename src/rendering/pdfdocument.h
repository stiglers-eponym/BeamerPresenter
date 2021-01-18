#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QDateTime>
#include <QUrl>
#include "src/rendering/pngpixmap.h"
/*
#ifdef INCLUDE_MUPDF
#include <mupdf/fitz/color.h>
#include <mupdf/fitz/transition.h>
#endif
*/

/// Unified type of PDF links for all PDF engines.
struct PdfLink {
    /// Positive values of type are interpreted as page numbers.
    /// Negative values are interpreted as LinkType.
    int type;
    QString target;
};

/// PDF outline (table of contents, TOC) entry for storing a tree in a list.
struct PdfOutlineEntry {
    /// Title of the entry.
    QString title;
    /// Page index in the PDF (start counting from 0, destination resolved by the PDF engine)
    int page;
    /// Index of next outline on the same level in some data structure.
    int next;
    /// Partial ordering defined by page number.
    //bool operator<(const PdfOutlineEntry& other) const {return page < other.page;}
    //bool operator<(const int other_page) const {return page < other_page;}
};

/// Unified type of slide transition for all PDF engines.
struct SlideTransition {
    /// Slide tansition Types.
    enum Type
    {
        Replace = 0,
/*
#ifdef INCLUDE_MUPDF
        // Allow static_cast from fz_transition::type.
        // It would also work with static cast in poppler, but that seems
        // quite unsafe (although they probably won't change the order).
        Split = FZ_TRANSITION_SPLIT,
        Blinds = FZ_TRANSITION_BLINDS,
        Box = FZ_TRANSITION_BOX,
        Wipe = FZ_TRANSITION_WIPE,
        Dissolve = FZ_TRANSITION_DISSOLVE,
        Glitter = FZ_TRANSITION_GLITTER,
        Fly = FZ_TRANSITION_FLY,
        Push = FZ_TRANSITION_PUSH,
        Cover = FZ_TRANSITION_COVER,
        Uncover = FZ_TRANSITION_UNCOVER,
        Fade = FZ_TRANSITION_FADE,
#else
*/
        Split,
        Blinds,
        Box,
        Wipe,
        Dissolve,
        Glitter,
        Fly,
        Push,
        Cover,
        Uncover,
        Fade,
        FlyRectangle,
    };

    /// Direction controlled by 2 bits for outwards and vertical.
    enum Properties
    {
        Outwards = 1,
        Vertical = 2,
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
};

/// Unified type of PDF video annotations for all PDF engines.
struct VideoAnnotation {
    QUrl file;
    enum Mode
    {
        Invalid = -1,
        Once = 0,
        Open,
        Palindrome,
        Repeat,
    } mode;
    QRectF rect;
};

/// Abstract class for handling PDF documents.
/// This class is inherited by classes specific for PDF engines.
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
    /// Backend / PDF engine
    enum PdfBackend {
#ifdef INCLUDE_POPPLER
        PopplerBackend = 0,
#endif
#ifdef INCLUDE_MUPDF
        MuPdfBackend = 1,
#endif
    };

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

    /// Label of page with given index.
    virtual const QString pageLabel(const int page) const = 0;

    /// Label of page with given index.
    virtual int pageIndex(const QString &page) const = 0;

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    virtual int overlaysShifted(const int start, const int shift_overlay) const = 0;

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    virtual QList<int> overlayIndices() const noexcept = 0;

    /// Check whether a file has been loaded successfully.
    virtual bool isValid() const = 0;

    /// Load the PDF outline, fill PdfDocument::outline.
    virtual void loadOutline() = 0;

    /// return outline
    const QVector<PdfOutlineEntry> &getOutline() const noexcept {return outline;}

    /// Return outline entry at given page.
    const PdfOutlineEntry &outlineEntryAt(const int page) const;

    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const = 0;

    /// Annotation at given position (in point = inch/72)
    virtual const VideoAnnotation annotationAt(const int page, const QPointF &position) const = 0;

    /// List all video annotations on given page. Returns NULL if list is
    /// empty.
    virtual QList<VideoAnnotation>* annotations(const int page) const = 0;

    /// Path to PDF file.
    const QString & getPath() const {return path;}

    /// Slide transition when reaching the given page.
    virtual const SlideTransition transition(const int page) const = 0;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept = 0;
};

#endif // PDFDOCUMENT_H
