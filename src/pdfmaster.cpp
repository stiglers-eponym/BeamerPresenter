// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/pdfmaster.h"

#include <zlib.h>

#include <QBuffer>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPainter>
#include <QRegularExpression>
#include <QStyleOptionGraphicsItem>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <algorithm>
#include <map>
#include <utility>

#include "src/config.h"
#include "src/drawing/pathcontainer.h"
#include "src/log.h"
#include "src/master.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/pdfdocument.h"
#include "src/slidescene.h"
#ifdef USE_QTPDF
#include "src/rendering/qtdocument.h"
#endif
#ifdef USE_POPPLER
#include "src/rendering/popplerdocument.h"
#endif
#ifdef USE_MUPDF
#include "src/rendering/mupdfdocument.h"
#endif

PdfMaster::~PdfMaster()
{
  qDeleteAll(paths);
  paths.clear();
}

bool PdfMaster::loadDocument(const QString &filename)
{
  if (document) {
    // Reload a document
    if (filename != document->getPath())
      preferences()->showErrorMessage(tr("Error while loading file"),
                                      tr("Tried to load a PDF file, but a "
                                         "different file is already loaded!"));
    else if (document->loadDocument()) {
      document->loadLabels();
      return true;
    }
    return false;
  }

  // Load a new document
  switch (preferences()->pdf_engine) {
#ifdef USE_POPPLER
    case PopplerEngine:
      document = std::shared_ptr<PdfDocument>(new PopplerDocument(filename));
      break;
#endif
#ifdef USE_MUPDF
    case MuPdfEngine:
      document = std::shared_ptr<PdfDocument>(new MuPdfDocument(filename));
      break;
#endif
#ifdef USE_QTPDF
    case QtPDFEngine:
      document = std::shared_ptr<PdfDocument>(new QtDocument(filename));
      break;
#endif
  }

  if (document == nullptr || !document->isValid()) {
    preferences()->showErrorMessage(tr("Error while loading file"),
                                    tr("Loading PDF document failed!"));
    return false;
  } else {
    document->loadLabels();
    return true;
  }
}

bool PdfMaster::loadDocument()
{
  if (document && document->loadDocument()) {
    document->loadLabels();
    return true;
  }
  return false;
}

template <class T>
QList<T *> PdfMaster::getActiveScenes(const PPage ppage) const
{
  QList<T *> list;
  for (auto scene : scenes) {
    if ((ppage.part == scene->pagePart()) &&
        (preferences()->overlay_mode == PerLabel
             ? overlaysShifted(scene->getPage(), FirstOverlay)
             : scene->getPage() == ppage.page))
      list.append(scene);
  }
  return list;
}

void PdfMaster::receiveAction(const Action action)
{
  switch (action) {
    case UndoDrawing:
    case UndoDrawingLeft:
    case UndoDrawingRight: {
      PPage ppage = {preferences()->page,
                     static_cast<PagePart>(action ^ UndoDrawing)};
      shiftToDrawings(ppage);
      PathContainer *const path = paths.value(ppage, nullptr);
      if (path) {
        debug_msg(DebugDrawing, "undo:" << path);
        const auto active_scenes = getActiveScenes<QGraphicsScene>(ppage);
        if (path->undo(active_scenes)) {
          _flags |= UnsavedDrawings;
          for (auto scene : active_scenes)
            // All entries of active_scenes have been casted from SlideScene* to
            // QGraphicsScene* before, so it is save to use static_cast here:
            static_cast<SlideScene *>(scene)->updateSelectionRect();
        }
      }
      break;
    }
    case RedoDrawing:
    case RedoDrawingLeft:
    case RedoDrawingRight: {
      PPage ppage = {preferences()->page,
                     static_cast<PagePart>(action ^ RedoDrawing)};
      shiftToDrawings(ppage);
      PathContainer *const path = paths.value(ppage, nullptr);
      if (path) {
        debug_msg(DebugDrawing, "redo:" << path);
        const auto active_scenes = getActiveScenes<QGraphicsScene>(ppage);
        if (path->redo(active_scenes)) {
          _flags |= UnsavedDrawings;
          for (auto scene : active_scenes)
            // All entries of active_scenes have been casted from SlideScene* to
            // QGraphicsScene* before, so it is save to use static_cast here:
            static_cast<SlideScene *>(scene)->updateSelectionRect();
        }
      }
      break;
    }
    case ClearDrawing:
    case ClearDrawingLeft:
    case ClearDrawingRight: {
      PPage ppage = {preferences()->page,
                     static_cast<PagePart>(action ^ RedoDrawing)};
      if (ppage.page >= 0 && preferences()->overlay_mode == PerLabel)
        ppage.page = document->overlaysShifted(ppage.page, FirstOverlay);
      PathContainer *const path = paths.value(ppage, nullptr);
      if (path) {
        debug_msg(DebugDrawing, "clear:" << path);
        if (path->clearPaths()) _flags |= UnsavedDrawings;
      }
      break;
    }
    default:
      break;
  }
}

void PdfMaster::replacePath(PPage ppage, QGraphicsItem *olditem,
                            QGraphicsItem *newitem)
{
  if (!olditem && !newitem) return;
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  paths[ppage]->replaceItem(olditem, newitem);
  _flags |= UnsavedDrawings;
}

void PdfMaster::addItemsForeground(PPage ppage,
                                   const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  paths[ppage]->addItemsForeground(items);
  _flags |= UnsavedDrawings;
}

void PdfMaster::removeItems(PPage ppage, const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  paths[ppage]->removeItems(items);
  _flags |= UnsavedDrawings;
}

void PdfMaster::addHistoryStep(
    PPage ppage, std::map<QGraphicsItem *, QTransform> *transforms,
    std::map<AbstractGraphicsPath *, drawHistory::DrawToolDifference> *tools,
    std::map<TextGraphicsItem *, drawHistory::TextPropertiesDifference> *texts)
{
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  if (paths[ppage]->addChanges(transforms, tools, texts))
    _flags |= UnsavedDrawings;
}

void PdfMaster::bringToForeground(PPage ppage,
                                  const QList<QGraphicsItem *> &to_foreground)
{
  if (to_foreground.empty()) return;
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  if (paths[ppage]->bringToForeground(to_foreground)) _flags |= UnsavedDrawings;
}

void PdfMaster::bringToBackground(PPage ppage,
                                  const QList<QGraphicsItem *> &to_background)
{
  if (to_background.empty()) return;
  shiftToDrawings(ppage);
  assertPageExists(ppage);
  if (paths[ppage]->bringToBackground(to_background)) _flags |= UnsavedDrawings;
}

void PdfMaster::distributeNavigationEvents(const int slide,
                                           const int page) const
{
  // Map (shifted) page numbers plus page parts to slide scenes.
  // Like this it can be detected if multiple scenes want to show the same
  // page. In this case the SlideViews showing the same page will all be
  // connected to the same scene.
  QMap<PPage, SlideScene *> scenemap;
  for (const auto scene : std::as_const(scenes)) {
    /// scenepage: the page index that will be shown by the scene.
    const int scenepage = overlaysShifted(page, scene->getShift());
    /// sceneslide: slide index of scene page
    const int sceneslide =
        scenepage == page ? slide : preferences()->slideForPage(scenepage);
    /// index page: the overlay root page, to which all drawings are attached.
    PPage indexpage = {scenepage, scene->pagePart()};
    if (preferences()->overlay_mode == PerLabel)
      indexpage.page = overlaysShifted(scenepage, FirstOverlay);
    if (scenemap.contains(indexpage))
      scene->navigationEvent(sceneslide, scenepage, scenemap[indexpage]);
    else {
      scenemap[indexpage] = scene;
      scene->navigationEvent(sceneslide, scenepage);
    }
  }
  for (const auto scene : std::as_const(scenes)) scene->createSliders();
}

void PdfMaster::writePages(QXmlStreamWriter &writer,
                           const bool save_bp_specific)
{
  QMap<PagePart, const PathContainer *> container_lst;
  QSizeF size;
  for (auto page : master()->pageIdx()) {
    container_lst.clear();
    size = document->pageSize(std::max(page, 0));
    for (const PagePart page_part : {FullPage, LeftHalf, RightHalf}) {
      PPage ppage = {page, page_part};
      shiftToDrawings(ppage);
      const PathContainer *container = paths.value(ppage, nullptr);
      if (container) container_lst[page_part] = container;
    }
    if (!container_lst.empty()) {
      QRectF drawing_rect = QRectF({0, 0}, size);
      for (auto container : container_lst)
        drawing_rect = drawing_rect.united(container->boundingBox());
      size = drawing_rect.size();
    }
    writer.writeStartElement("page");
    writer.writeAttribute("width", QString::number(size.width()));
    writer.writeAttribute("height", QString::number(size.height()));
    writer.writeEmptyElement("background");
    if (page >= 0) {
      writer.writeAttribute("type", "pdf");
      writer.writeAttribute("pageno", QString::number(page + 1));
      if (page == 0) {
        writer.writeAttribute("domain", "absolute");
        writer.writeAttribute("filename", document->getPath());
      }
    } else {
      writer.writeAttribute("type", "solid");
      writer.writeAttribute("style", "plain");
      writer.writeAttribute("color", "#ffffff00");
    }
    if (save_bp_specific && target_times.contains(page))
      writer.writeAttribute("endtime",
                            QTime::fromMSecsSinceStartOfDay(target_times[page])
                                .toString("h:mm:ss"));

    for (auto it = container_lst.cbegin(); it != container_lst.cend(); ++it) {
      writer.writeStartElement("layer");
      writer.writeAttribute("pagePart",
                            page_part_names.value(it.key(), "unknown"));
      (*it)->writeXml(writer);
      writer.writeEndElement();  // "layer" element
    }
    writer.writeEndElement();  // "page" element
  }
  _flags &= ~UnsavedDrawings;
  if (save_bp_specific) _flags &= ~UnsavedTimes;
}

QBuffer *loadZipToBuffer(const QString &filename)
{
  // This is probably not how it should be done.
  QBuffer *buffer = new QBuffer();
  buffer->open(QBuffer::ReadWrite);
  gzFile file = gzopen(filename.toUtf8(), "rb");
  if (!file) {
    qWarning() << "Loading drawings failed: file" << filename
               << "could not be opened";
    preferences()->showErrorMessage(
        PdfMaster::tr("Error while loading file"),
        PdfMaster::tr("Loading drawings failed: file ") + filename +
            PdfMaster::tr(" could not be opened"));
    return nullptr;
  }
  gzbuffer(file, 32768);
  char chunk[512];
  int status = 0;
  do {
    status = gzread(file, chunk, 512);
    buffer->write(chunk, status);
  } while (status == 512);
  gzclose_r(file);
  buffer->seek(0);
  return buffer;
}

void PdfMaster::readDrawingsFromStream(QXmlStreamReader &reader, const int page)
{
  if (page >= document->numberOfPages()) return;
  // TODO: check how to handle per-label drawings here!
  if ((_flags & HalfPageUsed) == 0) {
    PathContainer *container = paths.value({page, FullPage}, nullptr);
    if (!container) {
      container = new PathContainer(this);
      paths[{page, FullPage}] = container;
    }
    container->loadDrawings(reader);
    return;
  }
  PathContainer *left = paths.value({page, LeftHalf}, nullptr),
                *right = paths.value({page, RightHalf}, nullptr),
                *center = paths.value({page, FullPage}, nullptr);
  const qreal page_half = document->pageSize(std::max(page, 0)).width() / 2;
  if (_flags & LeftHalfUsed && !left) {
    left = new PathContainer(this);
    paths[{page, LeftHalf}] = left;
  }
  if (_flags & RightHalfUsed && !right) {
    right = new PathContainer(this);
    paths[{page, RightHalf}] = right;
  }
  if (_flags & FullPageUsed && !center) {
    center = new PathContainer(this);
    paths[{page, FullPage}] = center;
  }
  PathContainer::loadDrawings(reader, center, left, right, page_half);
}

PathContainer *PdfMaster::pathContainerCreate(PPage ppage)
{
  switch (preferences()->overlay_mode) {
    case PerPage:
      return paths.value(ppage, nullptr);
    case PerLabel:
      shiftToDrawings(ppage);
      return paths.value(ppage, nullptr);
    case Cumulative: {
      PathContainer *container = paths.value(ppage);
      if (container && !container->empty() && !container->isPlainCopy())
        return container;
      const int start_overlay =
          document->overlaysShifted(ppage.page, FirstOverlay);
      PathContainer *copy_container;
      int source_page = ppage.page;
      while (source_page-- > start_overlay) {
        copy_container = paths.value({source_page, ppage.part}, nullptr);
        if (copy_container) {
          delete container;
          container = copy_container->copy();
          paths[ppage] = container;
          return container;
        }
      }
      return nullptr;
    }
    default:
      return nullptr;
  }
}

void PdfMaster::createPathContainer(PathContainer **container, PPage ppage)
{
  shiftToDrawings(ppage);
  auto &target = paths[ppage];
  if (!target) target = new PathContainer(this);
  *container = target;
}

void PdfMaster::clearAllDrawings()
{
  for (const auto container : std::as_const(paths))
    if (container) container->clearPaths();
}

void PdfMaster::getTimeForPage(const int page, quint32 &time) const noexcept
{
  if (!target_times.empty() && page <= target_times.lastKey())
    time = *target_times.lowerBound(page);
}

bool PdfMaster::hasDrawings() const noexcept
{
  for (auto path : std::as_const(paths))
    if (!path->isCleared()) return true;
  return false;
}

void PdfMaster::search(const QString &text, const int &page, const bool forward)
{
  if (!document || page < 0) return;
  if (text == "") {
    search_results.second.clear();
    emit updateSearch();
    return;
  }
  search_results = document->searchAll(text, page, forward);
  if (search_results.first == preferences()->page)
    emit updateSearch();
  else if (search_results.first >= 0)
    emit navigationSignal(preferences()->slideForPage(search_results.first),
                          search_results.first);
}

QPixmap PdfMaster::exportImage(const PPage ppage,
                               const qreal resolution) const noexcept
{
  if (!document || resolution <= 0 || ppage.page >= document->numberOfPages())
    return QPixmap();
  debug_msg(DebugDrawing,
            "Export image" << ppage.page << ppage.part << resolution);
  QPixmap pixmap;
  if (ppage.page >= 0) {
    const auto *renderer = createRenderer(document, ppage.part);
    if (!renderer || !renderer->isValid()) return QPixmap();
    QPixmap pixmap = renderer->renderPixmap(ppage.page, resolution);
  } else {
    pixmap = QPixmap(document->pageSize(0).toSize());
  }
  const auto *container = paths.value(ppage, nullptr);
  if (container) {
    debug_msg(DebugDrawing, "Exporting items");
    QStyleOptionGraphicsItem style;
    QPainter painter;
    painter.begin(&pixmap);
    for (auto item : *container) {
      painter.resetTransform();
      painter.scale(resolution, resolution);
      painter.setTransform(item->sceneTransform(), true);
      item->paint(&painter, &style);
    }
    painter.end();
  }
  return pixmap;
}
