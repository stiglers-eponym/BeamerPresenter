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
    void loadPageLabels();

public:
    /// Create new document from given filename.
    MuPdfDocument(const QString &filename);
    ~MuPdfDocument() override;
    bool loadDocument() override;
    /// Size of page in points (72*inch).
    const QSizeF pageSize(const int page) const override;
    bool isValid() const override;
    const QString label(const int page) const override;
    int overlaysShifted(const int start, const int shift_overlay) const override;

    void loadLinks(const int page) const;

    int numberOfPages() const override
    {return number_of_pages;}

    fz_context* getContext() const
    {return ctx;}

    fz_document* getDocument() const
    {return doc;}

    /// Link at given position (in point = inch/72)
    virtual const PdfLink linkAt(const int page, const QPointF &position) const override;
    virtual const VideoAnnotation annotationAt(const int page, const QPointF &position) const override;
    virtual QList<VideoAnnotation>* annotations(const int page) const override;

    /// Prepare rendering for other threads by initializing the given pointers.
    /// This gives the threads only access to objects which are thread save.
    void prepareRendering(fz_context **context, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution) const;

    const SlideTransition transition(const int page) const override;
};

void lock_mutex(void *user, int lock);
void unlock_mutex(void *user, int lock);

struct label_item
{
    const char* style;
    const char* prefix;
    int start_value;
};

#endif // MUPDFDOCUMENT_H
