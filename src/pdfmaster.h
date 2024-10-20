// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PDFMASTER_H
#define PDFMASTER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QRectF>
#include <QString>
#include <algorithm>
#include <map>
#include <memory>
#include <utility>

#include "src/config.h"
#include "src/drawing/pathcontainer.h"
#include "src/enumerates.h"
#include "src/rendering/pdfdocument.h"

class SlideScene;
class QGraphicsItem;
class QBuffer;
class QXmlStreamReader;
class QXmlStreamWriter;
class AbstractGraphicsPath;
class TextGraphicsItem;
struct SlideTransition;

namespace drawHistory
{
struct DrawToolDifference;
struct TextPropertiesDifference;
struct ZValueChange;
struct Step;
}  // namespace drawHistory

/**
 * Full document including PDF and paths / annotations added by user.
 * This should also manage drawings and multimedia content of the PDF.
 */
class PdfMaster : public QObject
{
  Q_OBJECT

 public:
  /// Flags for different kinds of unsaved changes.
  enum PdfMasterFlag {
    UnsavedDrawings = 1 << 0,
    UnsavedTimes = 1 << 1,
    UnsavedNotes = 1 << 2,
    FullPageUsed = LeftHalf >> 1,
    LeftHalfUsed = LeftHalf,
    RightHalfUsed = RightHalf,
    HalfPageUsed = LeftHalf | RightHalf,
  };
  Q_DECLARE_FLAGS(PdfMasterFlags, PdfMasterFlag);
  Q_FLAG(PdfMasterFlags);

 private:
  /// Document representing the PDF
  std::shared_ptr<PdfDocument> document;

  /// Graphics scenes of this application. For each combination of PDF file
  /// and page shift one scene is created.
  /// The first scene in this list is the master scene.
  QList<SlideScene *> scenes;

  /// Path to file in which drawings are saved.
  QString drawings_path;

  /// Map page numbers and parts to containers of paths.
  /// Paths can be drawn per slide label by creating references to the main
  /// path list from other slide numbers.
  QMap<PPage, PathContainer *> paths;

  /// Time at which a slide should be finished.
  QMap<int, quint32> target_times;

  /// Flags for unsaved changes.
  PdfMasterFlags _flags = {};

  /// Search results (currently only one results)
  std::pair<int, QList<QRectF>> search_results;

  /// make sure paths[page] is a PathContainer*
  void assertPageExists(const PPage ppage) noexcept
  {
    if (!paths.value(ppage, nullptr)) paths[ppage] = new PathContainer(this);
  }

  /// List of scenes active on current page and given page part.
  template <class T>
  QList<T *> getActiveScenes(const PPage ppage) const;

  /// Shift page to first overlay if drawings are per label.
  void shiftToDrawings(PPage &ppage) const
  {
    if (ppage.page >= 0 && preferences()->overlay_mode == PerLabel)
      ppage.page = document->overlaysShifted(ppage.page, FirstOverlay);
  }

 public:
  /// Create empty, uninitialized PdfMaster.
  explicit PdfMaster() {}

  /// Destructor. Deletes paths and document.
  ~PdfMaster();

  /// get function for search_results
  const std::pair<int, QList<QRectF>> &searchResults() const noexcept
  {
    return search_results;
  }

  /// get function for _flags
  PdfMasterFlags &flags() noexcept { return _flags; }

  /// Load PDF file.
  bool loadDocument(const QString &filename);

  /// Load or reload the file. Return true if the file was updated and false
  /// otherwise.
  bool loadDocument();

  /// Get path to PDF file.
  const QString &getFilename() const { return document->getPath(); }

  /// Get the list of SlideScenes connected to this PDF.
  QList<SlideScene *> &getScenes() { return scenes; }

  /// Get size of page in points (floating point precision).
  const QSizeF getPageSize(const int page_number) const
  {
    return document->pageSize(std::max(page_number, 0));
  }

  /// Get PdfDocument.
  std::shared_ptr<PdfDocument> getDocument() const { return document; }

  /// Return true if document contains pages of different size.
  bool flexiblePageSizes() const noexcept
  {
    return document->flexiblePageSizes();
  }

  /// Clear history of given page.
  void clearHistory(const PPage ppage, const int remaining_entries) const
  {
    PathContainer *container = paths.value(ppage, nullptr);
    if (container) container->clearHistory(remaining_entries);
  }

  /// Slide transition when reaching the given page number.
  const SlideTransition transition(const int page) const
  {
    return document->transition(page);
  }

  /// Number of pages in the document.
  int numberOfPages() const { return document->numberOfPages(); }

  /// Get page number of start shifted by shift_overlay.
  /// Here in shift_overlay the bits of ShiftOverlay::FirstOverlay and
  /// ShiftOverlay::LastOverlay control the interpretation of the shift.
  /// Shifting with overlays means that only pages with a different page
  /// label (or own outline entry) start a new "real" slide.
  int overlaysShifted(const int start, const int shift_overlay) const
  {
    return document->overlaysShifted(start, shift_overlay);
  }

  /// Write page (part) to image, including drawings.
  QPixmap exportImage(const PPage ppage, const qreal resolution) const noexcept;

  /// Load drawings from XML reader, must be in element <layer>
  void readDrawingsFromStream(QXmlStreamReader &reader, const int page);

  /// Get path container at given page. If overlay_mode==Cumulative, this may
  /// create and return a copy of a previous path container.
  /// page (part) number is given as (page | page_part).
  PathContainer *pathContainerCreate(PPage ppage);

  /// Get path container at given page.
  /// page (part) number is given as (page | page_part).
  PathContainer *pathContainer(PPage ppage) const
  {
    if (preferences()->overlay_mode == PerLabel)
      ppage.page = overlaysShifted(ppage.page, FirstOverlay);
    return paths.value(ppage, nullptr);
  }

  bool hasPage(const int page) const
  {
    return paths.contains({page, FullPage}) ||
           paths.contains({page, LeftHalf}) ||
           paths.contains({page, RightHalf});
  }

  /// Get file path at which drawings are saved.
  const QString &drawingsPath() const noexcept { return drawings_path; }

  /// Clear all drawings but keep history.
  void clearAllDrawings();

  /// Check if page currently contains any drawings (ignoring history).
  bool hasDrawings() const noexcept;

  /// Write pages objects to XML
  void writePages(QXmlStreamWriter &writer, const bool save_bp_specific);

 public slots:
  /// Handle the given action.
  void receiveAction(const Action action);

  /// Add a new path (or QGraphicsItem) to paths[page].
  /// Page (part) number is given as (page | page_part).
  /// If item is nullptr: create the container if it does not exist yet.
  void receiveNewPath(const PPage ppage, QGraphicsItem *item)
  {
    replacePath(ppage, nullptr, item);
  }

  /// Replace an existing path (or QGraphicsItem) in paths[page] by the gievn
  /// new one. Old or new item can be nullptr, then only a new item will be
  /// created or an existing one will be removed, respectively. Page (part)
  /// number is given as (page | page_part). If both items are nullptr, only the
  /// container is created (if it doesn't exist yet).
  void replacePath(PPage ppage, QGraphicsItem *olditem, QGraphicsItem *newitem);

  /// Add history step with transformations, tool changes, and text
  /// property changes (not text content changes!).
  /// Page (part) number is given as (page | page_part).
  void addHistoryStep(
      PPage ppage, std::map<QGraphicsItem *, QTransform> *transforms,
      std::map<AbstractGraphicsPath *, drawHistory::DrawToolDifference> *tools,
      std::map<TextGraphicsItem *, drawHistory::TextPropertiesDifference>
          *texts);

  /// Add new paths.
  void addItemsForeground(const PPage ppage,
                          const QList<QGraphicsItem *> &items);
  /// Remove paths.
  void removeItems(const PPage ppage, const QList<QGraphicsItem *> &items);

  /// Send navigation events to all SlideScenes reading from this document.
  /// This is done centrally via PdfMaster because it may be necessary
  /// to reconnect SlideViews and SlideScenes if multiple scenes would
  /// show the same page.
  void distributeNavigationEvents(const int slide, const int page) const;

  /// Get path container at given page. If overlay_mode==Cumulative, this may
  /// create and return a copy of a previous path container.
  /// page (part) number is given as (page | page_part).
  void requestNewPathContainer(PathContainer **container, const PPage ppage)
  {
    *container = pathContainerCreate(ppage);
  }

  /// Get path container at given page. Always create a new container if it
  /// does not exist yet.
  void createPathContainer(PathContainer **container, const PPage ppage);

  /// Get target_times map reference
  QMap<int, quint32> &targetTimes() noexcept { return target_times; }

  /// Set time for page and write it to target_times.
  void setTimeForPage(const int page, const quint32 time) noexcept
  {
    target_times[page] = time;
    _flags |= UnsavedTimes;
  }

  /// Get time for given page and write it to time.
  void getTimeForPage(const int page, quint32 &time) const noexcept;

  /// Set UnsavedDrawings flag.
  void newUnsavedDrawings() noexcept { _flags |= UnsavedDrawings; }

  /// Bring given items to foreground and add history step.
  void bringToForeground(PPage ppage,
                         const QList<QGraphicsItem *> &to_foreground);
  /// Bring given items to background and add history step.
  void bringToBackground(PPage ppage,
                         const QList<QGraphicsItem *> &to_background);

  /// Handle the given action.
  void search(const QString &text, const int &page, const bool forward);

  /// change drawings_path.
  void setDrawingsPath(const QString &filename) noexcept
  {
    drawings_path = filename;
  }

 signals:
  /// Write notes from notes widgets to stream writer.
  void writeNotes(QXmlStreamWriter &writer) const;
  /// Read notes in notes widgets from stream reader.
  void readNotes(QXmlStreamReader &reader) const;
  /// Set total time of presentation (preferences().total_time).
  void setTotalTime(const QTime time) const;
  /// Send navigation signal to master.
  void sendPage(const int page);
  /// Tell slides to update search results.
  void updateSearch();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PdfMaster::PdfMasterFlags);

/// Unzip file to buffer.
QBuffer *loadZipToBuffer(const QString &filename);

#endif  // PDFMASTER_H
