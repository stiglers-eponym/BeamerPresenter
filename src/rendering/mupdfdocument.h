// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef MUPDFDOCUMENT_H
#define MUPDFDOCUMENT_H

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>
#include <QVector>
#include <memory>
#include <utility>

#include "src/config.h"
// old versions of MuPDF don't have 'extern "C"' in the header files.
extern "C" {
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
}
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"

class QMutex;
class QSizeF;
class QPointF;

/**
 * @brief MuPDF implementation of PdfDocument
 * @implements PdfDocument
 *
 * Document representing a PDF loaded by MuPDF.
 * MuPDF requires careful treatment of separte threads!
 *
 * @see PopplerDocument
 */
class MuPdfDocument : public PdfDocument
{
  Q_DECLARE_TR_FUNCTIONS(MuPdfDocument)

  static constexpr int max_search_results = 20;

#if (FZ_VERSION_MAJOR < 1) || \
    ((FZ_VERSION_MAJOR == 1) && (FZ_VERSION_MINOR < 22))
 public:
  /// Item in PDF PageLabel list. This is only used internally in
  /// MuPdfDocument::loadPageLabels().
  struct label_item {
    /// Page label style
    const char *style;
    /// Page label prefix
    const char *prefix;
    /// Page label start number
    int start_value;
  };
#endif  // FZ_VERSION < 1.22

 private:
  /// List of all pages.
  QVector<pdf_page *> pages;

  /// context should be cloned for each separate thread.
  fz_context *ctx{nullptr};

  /// document is global, don't clone if for threads.
  pdf_document *doc{nullptr};

  /// Mutexes needed for parallel rendering in MuPDF.
  QVector<QMutex *> mutex_list;

  /// Single mutex for this.
  QMutex *mutex;

  /// Total number of pages in document.
  int number_of_pages;

  /// Map of PDF object numbers to embedded media data streams
  QMap<int, std::shared_ptr<QByteArray>> embedded_media;

  /// populate pageLabels. Must be called after loadOutline.
  void loadPageLabels();

  /// Load the PDF outline, fill PdfDocument::outline.
  void loadOutline();

  /// Search for raw_needle on page and put results in target.
  /// Return number of matches.
  int searchPage(const int page, const char *raw_needle,
                 QList<QRectF> &target) const;

  void fillOutline(fz_outline *entry);

 public:
  /// Constructor: Create mutexes and load document using loadDocument().
  MuPdfDocument(const QString &filename);

  /// Destructor: delete mutexes, drop doc and ctx.
  ~MuPdfDocument() override;

  PdfEngine type() const noexcept override { return PdfEngine::MuPdf; }

  /// Load or reload the PDF document if the file has been modified since
  /// it was loaded. Return true if the document was reloaded.
  bool loadDocument() override final;

  /// Size of page in points (inch/72).
  const QSizeF pageSize(const int page) const override;

  /// Check whether a file has been loaded successfully.
  bool isValid() const noexcept override
  {
    return doc && ctx && number_of_pages > 0;
  }

  /// Duration of given page in secons. Default value is -1 is interpreted as
  /// infinity.
  qreal duration(const int page) const noexcept override;

  /// Total number of pages in document.
  int numberOfPages() const noexcept override { return number_of_pages; }

  /// Fitz context.
  fz_context *getContext() const noexcept { return ctx; }

  /// Fitz document.
  pdf_document *getDocument() const noexcept { return doc; }

  /// Load the PDF labels and outline, fill PdfDocument::outline.
  void loadLabels() override;

  /// Search which pages contain text.
  std::pair<int, QRectF> search(const QString &needle, int start_page,
                                bool forward) const override;

  /// Search which page contains needle and return the
  /// outline of all occurrences on that slide.
  std::pair<int, QList<QRectF>> searchAll(const QString &needle,
                                          int start_page = 0,
                                          bool forward = true) const override;

  /// Link at given position (in point = inch/72)
  virtual const PdfLink *linkAt(const int page,
                                const QPointF &position) const override;

  /// List all video annotations on given page.
  virtual QList<std::shared_ptr<MediaAnnotation>> annotations(
      const int page) override;

  /// Prepare rendering for other threads by initializing the given pointers.
  /// This gives the threads only access to objects which are thread save.
  void prepareRendering(fz_context **context, fz_rect *bbox,
                        fz_display_list **list, const int pagenumber,
                        const qreal resolution) const;

  /// Slide transition when reaching the given page.
  const SlideTransition transition(const int page) const override;

  /// Return true if not all pages in the PDF have the same size.
  virtual bool flexiblePageSizes() noexcept override;
};

/// Lock mutex <lock> in vector <user> of mutexes.
/// First argument must be of type QVector<QMutex*>*.
inline void lock_mutex(void *user, int lock)
{
  QVector<QMutex *> *mutex = static_cast<QVector<QMutex *> *>(user);
  (*mutex)[lock]->lock();
}

/// Lock mutex <lock> in vector <user> of mutexes.
/// First argument must be of type QVector<QMutex*>*.
inline void unlock_mutex(void *user, int lock)
{
  QVector<QMutex *> *mutex = static_cast<QVector<QMutex *> *>(user);
  (*mutex)[lock]->unlock();
}

#endif  // MUPDFDOCUMENT_H
