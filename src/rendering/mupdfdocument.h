#ifndef MUPDFDOCUMENT_H
#define MUPDFDOCUMENT_H

#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <QMutex>
#include <QObject>
#include <QFileInfo>
#include "src/rendering/pdfdocument.h"

/// Document representing a PDF loaded by MuPDF.
/// MuPDF requires careful treatment of separte threads!
class MuPdfDocument : public PdfDocument
{

    /// context should be cloned for each separate thread.
    fz_context *ctx{nullptr};

    /// document is global, don't clone if for threads.
    fz_document *doc{nullptr};

    /// Mutexes needed for parallel rendering in MuPDF.
    QVector<QMutex*> mutex_list{FZ_LOCK_MAX};

    /// Single mutex for this.
    QMutex* mutex;

    /// Total number of pages in document.
    int number_of_pages;

    /// Map page numbers to labels: Only the first page number with a new label
    /// is listed here.
    QMap<int, QString> pageLabels;

    /// populate pageLabels.
    void loadPageLabels();

public:
    /// Constructor: Create mutexes and load document using loadDocument().
    MuPdfDocument(const QString &filename);

    /// Destructor: delete mutexes, drop doc and ctx.
    ~MuPdfDocument() override;

    /// Load or reload the PDF document if the file has been modified since
    /// it was loaded. Return true if the document was reloaded.
    bool loadDocument() override;

    /// Size of page in points (inch/72).
    const QSizeF pageSize(const int page) const override;

    /// Check whether a file has been loaded successfully.
    bool isValid() const override
    {return doc && ctx && number_of_pages > 0;}

    /// Label of given page. This currently only supports numerical values.
    const QString pageLabel(const int page) const override;

    /// Label of page with given index.
    int pageIndex(const QString &page) const override;

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// Total number of pages in document.
    int numberOfPages() const override
    {return number_of_pages;}

    /// Fitz context.
    fz_context* getContext() const
    {return ctx;}

    /// Fitz document.
    fz_document* getDocument() const
    {return doc;}

    /// Load the PDF outline, fill PdfDocument::outline.
    void loadOutline() override;

    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const override;

    /// Annotation at given position (in point = inch/72)
    virtual const VideoAnnotation annotationAt(const int page, const QPointF &position) const override;

    /// List all video annotations on given page. Returns nullptr if list is
    /// empty.
    virtual QList<VideoAnnotation>* annotations(const int page) const override;

    /// Prepare rendering for other threads by initializing the given pointers.
    /// This gives the threads only access to objects which are thread save.
    void prepareRendering(fz_context **context, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution) const;

    /// Slide transition when reaching the given page.
    const SlideTransition transition(const int page) const override;

    /// Return true if not all pages in the PDF have the same size.
    virtual bool flexiblePageSizes() noexcept override;
};

/// Lock mutex <lock> in vector <user> of mutexes.
/// First argument must be of type QVector<QMutex*>*.
void lock_mutex(void *user, int lock);

/// Lock mutex <lock> in vector <user> of mutexes.
/// First argument must be of type QVector<QMutex*>*.
void unlock_mutex(void *user, int lock);

/// Item in PDF PageLabel list. This is only used internally in
/// MuPdfDocument::loadPageLabels().
struct label_item
{
    const char* style;
    const char* prefix;
    int start_value;
};

#endif // MUPDFDOCUMENT_H
