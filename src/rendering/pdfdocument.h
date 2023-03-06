// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

#include <QString>
#include <QDateTime>
#include <QUrl>
#include <QRectF>
#include <QByteArray>
#include <QVector>
#include "src/config.h"
#include "src/enumerates.h"


/// Unified type of PDF media annotations for all PDF engines.
struct MediaAnnotation {
    /// URL for (external) media file.
    QUrl file;
    /// Media type. Embedded types are currently not supported.
    enum Type
    {
        /// not initialized or invalid
        InvalidAnnotation = 0,
        /// flag for media annnotation that has audio
        HasAudio = 1 << 0,
        /// flag for media annnotation that has video
        HasVideo = 1 << 1,
        /// flag for embedded media annnotation. Not embedded means external file.
        Embedded = 1 << 2,
        /// embedded video (doesn't exist, but why not included it here)
        VideoEmbedded = HasVideo | HasAudio | Embedded,
        /// embedded audio (currently not supported)
        AudioEmbedded = HasAudio | Embedded,
        /// external video (with audio)
        VideoExternal = HasVideo | HasAudio,
        /// external audio-only file
        AudioExternal = HasAudio,
    };
    /// Media type.
    Type type = InvalidAnnotation;

    /// Play modes of media.
    enum Mode
    {
        /// Invlid mode
        InvalidMode = -1,
        /// Play media only once
        Once = 0,
        /// Play video and show controll bar. Currently ignored.
        Open,
        /// Play continuously forward and backward. Currently not implemented.
        Palindrome,
        /// Play repeatedly (infinite loop).
        Repeat,
    };
    /// Play mode
    Mode mode = Once;

    /// Audio volume of media.
    float volume = 1.;
    /// Position of media on slide.
    QRectF rect;
    /// Trivial constructor.
    MediaAnnotation() : file(), type(InvalidAnnotation), mode(InvalidMode), rect() {}
    /// Constructor for full initialization.
    MediaAnnotation(const QUrl &url, const bool hasvideo, const QRectF &rect) :
        file(url), type(hasvideo ? VideoExternal : AudioExternal), mode(Once), rect(rect) {}
    /// Constructor without file.
    MediaAnnotation(const Type type, const QRectF &rect) :
        file(QUrl()), type(type), mode(Once), rect(rect) {}
    /// Trivial destructor.
    virtual ~MediaAnnotation() {}
    /// Comparison by media type, file, mode, and rect.
    virtual bool operator==(const MediaAnnotation &other) const noexcept
    {return     type == other.type
                && file == other.file
                && mode == other.mode
                && rect == other.rect;}
};

/// Embedded media file.
/// Quite useless because currently these objects cannot be played.
/// @todo implement embedded media files.
struct EmbeddedMedia : MediaAnnotation {
    /// Data stream.
    QByteArray data;
    /// Audio sampling rate
    int sampling_rate;
    /// Audio channels
    int channels = 1;
    /// Bit per sample
    int bit_per_sample = 8;

    /// Audio encoding modes as defined by PDF standard
    enum Encoding {
        /// Raw unsigned integers between 0 and 2^8-1
        SoundEncodingRaw,
        /// Twos-complement values
        SoundEncodingSigned,
        /// mu-law encoded samples
        SoundEncodingMuLaw,
        /// A-law-encoded samples
        SoundEncodingALaw,
    }
    /// Audio encoding
    encoding = SoundEncodingRaw;

    /// Stream compression modes
    enum Compression {
        /// no compression
        Uncompressed,
    }
    /// Stream compression
    compression = Uncompressed;

    /// Constructor
    EmbeddedMedia(const QByteArray &data, int sampling_rate, const QRectF &rect) :
        MediaAnnotation(AudioEmbedded, rect), data(data), sampling_rate(sampling_rate) {}
    /// Trivial destructor
    virtual ~EmbeddedMedia() {}
    /// Comparison by all properties, including data.
    virtual bool operator==(const MediaAnnotation &other) const noexcept override
    {return     type == other.type
                && file == other.file
                && data.data() == static_cast<const EmbeddedMedia&>(other).data.data()
                && mode == other.mode
                && rect == other.rect;}
};

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
struct ExternalLink : PdfLink {
    QUrl url;
    ExternalLink(const LinkType type, const QRectF &area, const QUrl &url) : PdfLink(type, area), url(url) {}
};
struct GotoLink : PdfLink {
    int page;
    GotoLink(const QRectF &area, const int page) : PdfLink(PageLink, area), page(page) {}
};
struct ActionLink : PdfLink {
    Action action;
    ActionLink(const QRectF &area, const Action action) : PdfLink(LinkType::ActionLink, area), action(action) {}
};
struct MediaLink : PdfLink {
    MediaAnnotation annotation;
    MediaLink(const LinkType type, const QRectF &area, const MediaAnnotation &annotation) : PdfLink(type, area), annotation(annotation) {}
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
    {properties ^= Outwards; angle = (angle + 180) % 360;}
};

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

    /// Label of page with given index.
    /// The default implementation returns a string representing the page number.
    virtual const QString pageLabel(const int page) const
    {return QString::number(page+1);}

    /// Label of page with given index.
    virtual int pageIndex(const QString &page) const
    {return page.toInt() - 1;}

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

    /// Search which pages contain text.
    virtual QPair<int,QRectF> search(const QString &needle, int start_page = 0, bool forward = true) const
    {return {-1,QRectF()};}

    /// get function for outline
    const QVector<PdfOutlineEntry> &getOutline() const noexcept
    {return outline;}

    /// Return outline entry at given page.
    const PdfOutlineEntry &outlineEntryAt(const int page) const;

    /// Link at given position (in point = inch/72)
    virtual const PdfLink *linkAt(const int page, const QPointF &position) const
    {return nullptr;}

    /// List all video annotations on given page. Returns NULL if list is
    /// empty.
    virtual QList<MediaAnnotation>* annotations(const int page) const
    {return nullptr;}

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
