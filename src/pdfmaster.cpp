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

void PdfMaster::receiveAction(const Action action)
{
  switch (action) {
    case UndoDrawing:
    case UndoDrawingLeft:
    case UndoDrawingRight: {
      int page = preferences()->page;
      if (page >= 0 && preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted(page, FirstOverlay);
      page = (page & ~NotFullPage) | (action ^ UndoDrawing);
      PathContainer *const path = paths.value(page, nullptr);
      if (path) {
        debug_msg(DebugDrawing, "undo:" << path);
        auto scene_it = scenes.cbegin();
        if (preferences()->overlay_mode == PerLabel)
          while (scene_it != scenes.cend() &&
                 (overlaysShifted((*scene_it)->getPage(), FirstOverlay) |
                  (*scene_it)->pagePart()) != page)
            ++scene_it;
        else
          while (scene_it != scenes.cend() &&
                 ((*scene_it)->getPage() | (*scene_it)->pagePart()) != page)
            ++scene_it;
        SlideScene *const scene =
            scene_it == scenes.cend() ? nullptr : *scene_it;
        if (path->undo(scene)) {
          _flags |= UnsavedDrawings;
          if (scene) scene->updateSelectionRect();
        }
      }
      break;
    }
    case RedoDrawing:
    case RedoDrawingLeft:
    case RedoDrawingRight: {
      int page = preferences()->page;
      if (page >= 0 && preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted(page, FirstOverlay);
      page = (page & ~NotFullPage) | (action ^ RedoDrawing);
      PathContainer *const path = paths.value(page, nullptr);
      if (path) {
        debug_msg(DebugDrawing, "redo:" << path);
        auto scene_it = scenes.cbegin();
        if (preferences()->overlay_mode == PerLabel)
          while (scene_it != scenes.cend() &&
                 (overlaysShifted((*scene_it)->getPage(), FirstOverlay) |
                  (*scene_it)->pagePart()) != page)
            ++scene_it;
        else
          while (scene_it != scenes.cend() &&
                 ((*scene_it)->getPage() | (*scene_it)->pagePart()) != page)
            ++scene_it;
        SlideScene *const scene =
            scene_it == scenes.cend() ? nullptr : *scene_it;
        if (path->redo(scene)) {
          _flags |= UnsavedDrawings;
          if (scene) scene->updateSelectionRect();
        }
      }
      break;
    }
    case ClearDrawing:
    case ClearDrawingLeft:
    case ClearDrawingRight: {
      int page = preferences()->page;
      if (page >= 0 && preferences()->overlay_mode == PerLabel)
        page = document->overlaysShifted(page, FirstOverlay);
      page = (page & ~NotFullPage) | (action ^ ClearDrawing);
      PathContainer *const path = paths.value(page, nullptr);
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

void PdfMaster::replacePath(int page, QGraphicsItem *olditem,
                            QGraphicsItem *newitem)
{
  if (!olditem && !newitem) return;
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  paths[page]->replaceItem(olditem, newitem);
  _flags |= UnsavedDrawings;
}

void PdfMaster::addItemsForeground(int page,
                                   const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  paths[page]->addItemsForeground(items);
  _flags |= UnsavedDrawings;
}

void PdfMaster::removeItems(int page, const QList<QGraphicsItem *> &items)
{
  if (items.empty()) return;
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  paths[page]->removeItems(items);
  _flags |= UnsavedDrawings;
}

void PdfMaster::addHistoryStep(
    int page, std::map<QGraphicsItem *, QTransform> *transforms,
    std::map<AbstractGraphicsPath *, drawHistory::DrawToolDifference> *tools,
    std::map<TextGraphicsItem *, drawHistory::TextPropertiesDifference> *texts)
{
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  if (paths[page]->addChanges(transforms, tools, texts))
    _flags |= UnsavedDrawings;
}

void PdfMaster::bringToForeground(int page,
                                  const QList<QGraphicsItem *> &to_foreground)
{
  if (to_foreground.empty()) return;
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  if (paths[page]->bringToForeground(to_foreground)) _flags |= UnsavedDrawings;
}

void PdfMaster::bringToBackground(int page,
                                  const QList<QGraphicsItem *> &to_background)
{
  if (to_background.empty()) return;
  if (page >= 0 && preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  assertPageExists(page);
  if (paths[page]->bringToBackground(to_background)) _flags |= UnsavedDrawings;
}

void PdfMaster::distributeNavigationEvents(const int slide,
                                           const int page) const
{
  // Map (shifted) page numbers with page parts to slide scenes.
  // Like this it can be detected if multiple scenes want to show the same
  // page. In this case the SlideViews showing the same page will all be
  // connected to the same scene.
  QMap<int, SlideScene *> scenemap;
  if (preferences()->overlay_mode == PerLabel) {
    int scenepage, indexpage, sceneslide;
    for (const auto scene : std::as_const(scenes)) {
      /// scenepage: the page index that will be shown by the scene.
      scenepage = overlaysShifted(page, scene->getShift());
      sceneslide =
          scenepage == page ? slide : preferences()->slideForPage(scenepage);
      /// index page: the overlay root page, to which all drawings are attached.
      indexpage = overlaysShifted(scenepage, FirstOverlay) | scene->pagePart();
      if (scenemap.contains(indexpage))
        scene->navigationEvent(sceneslide, scenepage, scenemap[indexpage]);
      else {
        scenemap[indexpage] = scene;
        scene->navigationEvent(sceneslide, scenepage);
      }
      scenepage |= scene->pagePart();
    }
  } else {
    // Redistribute views to scenes.
    int scenepage, sceneslide;
    for (const auto scene : std::as_const(scenes)) {
      scenepage = overlaysShifted(page, scene->getShift()) | scene->pagePart();
      sceneslide = scenepage == (page & ~NotFullPage)
                       ? slide
                       : preferences()->slideForPage(scenepage);
      if (scenemap.contains(scenepage))
        scene->navigationEvent(sceneslide, scenepage & ~NotFullPage,
                               scenemap[scenepage]);
      else
        scenemap[scenepage] = scene;
    }
    // Navigation events to active scenes.
    for (auto it = scenemap.cbegin(); it != scenemap.cend(); ++it)
      (*it)->navigationEvent(sceneslide, it.key() & ~NotFullPage);
  }
  for (const auto scene : std::as_const(scenes)) scene->createSliders();
}

void PdfMaster::writePages(QXmlStreamWriter &writer,
                           const bool save_bp_specific)
{
  PathContainer *container;
  for (int i = 0; i < numberOfPages(); i++) {
    QSizeF size = document->pageSize(i);
    QRectF drawing_rect;
    for (const PagePart page_part : {FullPage, LeftHalf, RightHalf}) {
      if (preferences()->overlay_mode == PerLabel)
        container = paths.value(
            document->overlaysShifted(i | page_part, FirstOverlay), nullptr);
      else
        container = paths.value(i | page_part, nullptr);
      if (container)
        drawing_rect = drawing_rect.united(container->boundingBox());
    }
    size.setWidth(std::max(size.width(), drawing_rect.right()));
    size.setHeight(std::max(size.height(), drawing_rect.bottom()));
    writer.writeStartElement("page");
    writer.writeAttribute("width", QString::number(size.width()));
    writer.writeAttribute("height", QString::number(size.height()));
    writer.writeEmptyElement("background");
    writer.writeAttribute("type", "pdf");
    writer.writeAttribute("pageno", QString::number(i + 1));
    if (i == 0) {
      writer.writeAttribute("domain", "absolute");
      writer.writeAttribute("filename", document->getPath());
    }
    if (save_bp_specific && target_times.contains(i))
      writer.writeAttribute(
          "endtime",
          QTime::fromMSecsSinceStartOfDay(target_times[i]).toString("h:mm:ss"));

    writer.writeStartElement("layer");
    for (const auto page_part : {FullPage, LeftHalf, RightHalf}) {
      if (preferences()->overlay_mode == PerLabel)
        container = paths.value(
            document->overlaysShifted(i | page_part, FirstOverlay), nullptr);
      else
        container = paths.value(i | page_part, nullptr);
      if (container) container->writeXml(writer);
    }
    writer.writeEndElement();  // "layer" element
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

void PdfMaster::readPageFromStream(QXmlStreamReader &reader)
{
  debug_msg(DebugDrawing, "read page from stream" << reader.name());
  int page = -1;
  static const QRegularExpression regexpr_2nondigits("[^0-9]{2,2}$");
  while (reader.readNextStartElement()) {
    debug_msg(DebugDrawing, "Searching background" << reader.name());
    if (reader.name().toUtf8() == "background") {
      QString string = reader.attributes().value("pageno").toString();
      // For some reason Xournal++ adds "ll" as a sufix to the page number.
      if (string.contains(regexpr_2nondigits)) string.chop(2);
      bool ok;
      page = string.toInt(&ok) - 1;
      if (!ok) return;
      string = reader.attributes().value("endtime").toString();
      if (!string.isEmpty()) {
        const QTime time = QTime::fromString(string, "h:mm:ss");
        if (time.isValid()) target_times[page] = time.msecsSinceStartOfDay();
      }
#if (QT_VERSION_MAJOR >= 6)
      const QStringView filename = reader.attributes().value("filename");
#else
      const QStringRef filename = reader.attributes().value("filename");
#endif
      if (!filename.isEmpty() && filename != document->getPath() &&
          QFileInfo(filename.toString()).absoluteFilePath() !=
              document->getPath())
        return;
      if (!reader.isEndElement()) reader.skipCurrentElement();
      break;
    }
    if (!reader.isEndElement()) reader.skipCurrentElement();
  }
  if (page < 0) return;
  while (reader.readNextStartElement()) {
    if (reader.name().toUtf8() == "layer") readDrawingsFromStream(reader, page);
    if (!reader.isEndElement()) reader.skipCurrentElement();
  }
  reader.skipCurrentElement();
}

void PdfMaster::readDrawingsFromStream(QXmlStreamReader &reader, const int page)
{
  if (page < 0 || page >= document->numberOfPages()) return;
  if ((_flags & HalfPageUsed) == 0) {
    PathContainer *container = paths.value(page, nullptr);
    if (!container) {
      container = new PathContainer(this);
      paths[page] = container;
    }
    container->loadDrawings(reader);
    return;
  }
  PathContainer *left = paths.value(page | LeftHalf, nullptr),
                *right = paths.value(page | RightHalf, nullptr),
                *center = paths.value(page, nullptr);
  const qreal page_half = document->pageSize(page).width() / 2;
  if (_flags & LeftHalfUsed && !left) {
    left = new PathContainer(this);
    paths[page | LeftHalf] = left;
  }
  if (_flags & RightHalfUsed && !right) {
    right = new PathContainer(this);
    paths[page | RightHalf] = right;
  }
  if (_flags & FullPageUsed && !center) {
    center = new PathContainer(this);
    paths[page] = center;
  }
  PathContainer::loadDrawings(reader, center, left, right, page_half);
}

PathContainer *PdfMaster::pathContainerCreate(int page)
{
  switch (preferences()->overlay_mode) {
    case PerPage:
      return paths.value(page, nullptr);
    case PerLabel:
      page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
             (page & NotFullPage);
      return paths.value(page, nullptr);
    case Cumulative: {
      PathContainer *container = paths.value(page);
      if (container && !container->empty() && !container->isPlainCopy())
        return container;
      const int page_part = page & NotFullPage;
      int basic_page = page ^ page_part;
      const int start_overlay =
          document->overlaysShifted(basic_page, FirstOverlay);
      PathContainer *copy_container;
      while (basic_page-- > start_overlay) {
        copy_container = paths.value(basic_page | page_part, nullptr);
        if (copy_container) {
          delete container;
          container = copy_container->copy();
          paths[page] = container;
          return container;
        }
      }
      return nullptr;
    }
    default:
      return nullptr;
  }
}

void PdfMaster::createPathContainer(PathContainer **container, int page)
{
  if (preferences()->overlay_mode == PerLabel)
    page = document->overlaysShifted((page & ~NotFullPage), FirstOverlay) |
           (page & NotFullPage);
  auto &target = paths[page];
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

QPixmap PdfMaster::exportImage(const int page,
                               const qreal resolution) const noexcept
{
  if (!document || resolution <= 0 || page < 0 ||
      page >= document->numberOfPages())
    return QPixmap();
  debug_msg(DebugDrawing, "Export image" << page << resolution);
  const auto *renderer = createRenderer(
      document, static_cast<PagePart>(page & PagePart::NotFullPage));
  if (!renderer || !renderer->isValid()) return QPixmap();
  QPixmap pixmap =
      renderer->renderPixmap(page & ~PagePart::NotFullPage, resolution);
  const auto *container = paths.value(page, nullptr);
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
