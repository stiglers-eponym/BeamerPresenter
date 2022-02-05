#ifndef MUPDFDOCUMENT_H
#define MUPDFDOCUMENT_H


extern "C"
{
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
#include <mupdf/fitz/version.h>
}

#include <QMap>
#include "src/rendering/pdfdocument.h"

class QMutex;

/**
 * @brief MuPDF implementation of PdfDocument
 *
 * Document representing a PDF loaded by MuPDF.
 * MuPDF requires careful treatment of separte threads!
 */
class MuPdfDocument : public PdfDocument
{
public:
    /// Item in PDF PageLabel list. This is only used internally in
    /// MuPdfDocument::loadPageLabels().
    struct label_item
    {
        /// Page label style
        const char* style;
        /// Page label prefix
        const char* prefix;
        /// Page label start number
        int start_value;
    };

private:
    /// List of all pages.
    QVector<pdf_page*> pages;

    /// context should be cloned for each separate thread.
    fz_context *ctx{NULL};

    /// document is global, don't clone if for threads.
    pdf_document *doc{NULL};

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
    bool loadDocument() override final;

    /// Size of page in points (inch/72).
    const QSizeF pageSize(const int page) const override;

    /// Check whether a file has been loaded successfully.
    bool isValid() const override
    {return doc && ctx && number_of_pages > 0;}

    /// Label of given page. This currently only supports numerical values.
    const QString pageLabel(const int page) const override;

    /// Duration of given page in secons. Default value is -1 is interpreted as infinity.
    qreal duration(const int page) const noexcept override;

    /// Label of page with given index.
    int pageIndex(const QString &page) const override;

    /// Starting from page start, get the number (index) of the page shifted
    /// by shift_overlay.
    /// If shift is an int and overlay is of type ShiftOverlays:
    /// shift_overlay = (shift & ~AnyOverlay) | overlay
    /// overlay = shift & AnyOverlay
    /// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
    int overlaysShifted(const int start, const int shift_overlay) const override;

    /// List of indices, at which slide labels change. An empty list indicates
    /// that all consecutive slides have different labels.
    virtual QList<int> overlayIndices() const noexcept override
    {return pageLabels.size() == number_of_pages ? QList<int>() : pageLabels.keys();}

    /// Total number of pages in document.
    int numberOfPages() const override
    {return number_of_pages;}

    /// Fitz context.
    fz_context* getContext() const
    {return ctx;}

    /// Fitz document.
    pdf_document* getDocument() const
    {return doc;}

    /// Load the PDF outline, fill PdfDocument::outline.
    void loadOutline() override;

    /// Link at given position (in point = inch/72)
    virtual const PdfLink *linkAt(const int page, const QPointF &position) const override;

    /// List all video annotations on given page. Returns NULL if list is
    /// empty.
    virtual QList<MediaAnnotation>* annotations(const int page) const override;

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

#endif // MUPDFDOCUMENT_H
