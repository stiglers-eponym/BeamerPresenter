#ifndef MUPDFDOCUMENT_H
#define MUPDFDOCUMENT_H

#include <mupdf/fitz.h>
#include <QMutex>
#include <QObject>
#include <QFileInfo>
#include "src/rendering/pdfdocument.h"

class MuPdfDocument : public QObject, public PdfDocument
{
    Q_OBJECT

    /// context should be cloned for each separate thread.
    fz_context *context{nullptr};
    /// document is global, don't clone if for threads.
    fz_document *doc{nullptr};
    /// Mutexes needed for parallel rendering in MuPDF.
    QVector<QMutex*> mutex{FZ_LOCK_MAX};
    /// Total number of pages in document.
    int number_of_pages;

public:
    MuPdfDocument(const QString &filename);
    ~MuPdfDocument() override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const QPixmap getPixmap(const int page, const qreal resolution) const override;
    /// page is given as page index. resolution is given in pixels per point (72*dpi).
    const PngPixmap* getPng(const int page, const qreal resolution) const override;
    bool loadDocument() override;
    /// Size of page in points (72*inch).
    const QSizeF pageSize(const int page) const override;
    int numberOfPages() const override {return number_of_pages;}
    bool isValid() const override;
    const QString label(const int page) const override;
    fz_context* getContext() const {return context;}
    fz_document* getDocument() const {return doc;}

public slots:
    /// Prepare rendering for other threads.
    /// This gives the threads only access to objects which are thread save.
    void prepareRendering(fz_context **ctx, fz_rect *bbox, fz_display_list **list, const int pagenumber, const qreal resolution);
};

void lock_mutex(void *user, int lock);
void unlock_mutex(void *user, int lock);

#endif // MUPDFDOCUMENT_H
