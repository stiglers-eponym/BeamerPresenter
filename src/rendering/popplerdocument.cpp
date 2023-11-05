// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QBuffer>
#include <QByteArray>
#include <QFileInfo>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QUrl>
#include <QVector>
#include <QtConfig>
#include <utility>

#if (QT_VERSION_MAJOR == 6)
#include "poppler/qt6/poppler-media.h"
#elif (QT_VERSION_MAJOR == 5)
#include "poppler/qt5/poppler-media.h"
#endif

#include "src/log.h"
#include "src/preferences.h"
#include "src/rendering/pngpixmap.h"
#include "src/rendering/popplerdocument.h"
#include "src/rendering/popplerrenderer.h"

EmbeddedAudio *embeddedSound(const Poppler::SoundObject *sound,
                             const QRectF &rect)
{
  if (sound->soundType() != Poppler::SoundObject::Embedded) return nullptr;
  std::shared_ptr<QByteArray> data =
      std::make_shared<QByteArray>(sound->data());
  EmbeddedAudio *media = new EmbeddedAudio(data, sound->samplingRate(), rect);
  media->channels = sound->channels();
  media->bits_per_sample = sound->bitsPerSample();
  switch (sound->soundEncoding()) {
    case Poppler::SoundObject::Raw:
      media->encoding = EmbeddedAudio::SoundEncodingRaw;
      break;
    case Poppler::SoundObject::ALaw:
      media->encoding = EmbeddedAudio::SoundEncodingALaw;
      break;
    case Poppler::SoundObject::muLaw:
      media->encoding = EmbeddedAudio::SoundEncodingMuLaw;
      break;
    case Poppler::SoundObject::Signed:
      media->encoding = EmbeddedAudio::SoundEncodingSigned;
      break;
  }
  debug_msg(DebugMedia, "created embedded audio obejct" << media);
  return media;
}

PopplerDocument::PopplerDocument(const QString &filename)
    : PdfDocument(filename)
{
  // Load the document
  if (!loadDocument()) qFatal("Loading document failed");
  debug_msg(DebugRendering, "Loaded PDF document in Poppler");
}

int PopplerDocument::pageIndex(const QString &label) const
{
  // Fastest way: if pageLabels is empty, that means that we may simply
  // convert the string to an integer.
  if (pageLabels.isEmpty()) return label.toInt() - 1;
  // Next try Poppler's bulit-in function. This sometimes fails,
  // maybe because of an encoding problem?
  const std::unique_ptr<Poppler::Page> docpage(doc->page(label));
  if (docpage) return docpage->index();
  // If previous attempts fail, do something slow (linear time):
  return pageLabels.key(label, -1);
}

bool PopplerDocument::loadDocument()
{
  // Check if the file exists.
  const QFileInfo fileinfo(path);
  if (!fileinfo.exists() || !fileinfo.isFile()) {
    preferences()->showErrorMessage(
        tr("Error while loading file"),
        tr("Given filename is not a file: ") + fileinfo.baseName());
    return false;
  }
  // Check if the file has changed since last (re)load
  if (doc != nullptr && fileinfo.lastModified() == lastModified) return false;

  // Load the document.
  std::unique_ptr<Poppler::Document> newdoc(Poppler::Document::load(path));
  if (newdoc == nullptr) {
    qCritical() << tr("Failed to load document.");
    return false;
  }

  // Try to unlock a locked document.
  if (newdoc->isLocked()) {
    qWarning() << "Document is locked.";
    // Use a QInputDialog to ask for the password.
    bool ok;
    QString const password = QInputDialog::getText(
        nullptr, tr("Document is locked!"),
        tr("Please enter password (leave empty to cancel)."),
        QLineEdit::Password, QString(), &ok);
    // Check if a password was entered and try to unlock document.
    // Only user password is required, since we only read the document.
    if (!ok || password.isEmpty() ||
        newdoc->unlock(QByteArray(), password.toUtf8())) {
      preferences()->showErrorMessage(
          tr("Error while loading file"),
          tr("No or invalid password provided for locked document"));
      newdoc = nullptr;
      return false;
    }
  }
  // Save the modification time.
  lastModified = fileinfo.lastModified();

  // Set rendering hints.
  newdoc->setRenderHint(Poppler::Document::TextAntialiasing);
  newdoc->setRenderHint(Poppler::Document::TextHinting);
  newdoc->setRenderHint(Poppler::Document::TextSlightHinting);
  newdoc->setRenderHint(Poppler::Document::Antialiasing);
  newdoc->setRenderHint(Poppler::Document::ThinLineShape);

  // Update document and delete old document.
  if (newdoc != nullptr) doc.swap(newdoc);
  flexible_page_sizes = -1;

  return true;
}

const QPixmap PopplerDocument::getPixmap(const int page, const qreal resolution,
                                         const PagePart page_part) const
{
  const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
  if (resolution <= 0 || !docpage) {
    qWarning() << "Tried to render invalid page or invalid resolution" << page;
    return QPixmap();
  }
  const QImage image =
      docpage->renderToImage(72. * resolution, 72. * resolution);
  switch (page_part) {
    case LeftHalf:
      return QPixmap::fromImage(
          image.copy(0, 0, image.width() / 2, image.height()));
    case RightHalf:
      return QPixmap::fromImage(image.copy((image.width() + 1) / 2, 0,
                                           image.width() / 2, image.height()));
    default:
      return QPixmap::fromImage(image);
  }
}

const PngPixmap *PopplerDocument::getPng(const int page, const qreal resolution,
                                         const PagePart page_part) const
{
  const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
  if (resolution <= 0 || !docpage) {
    qWarning() << "Tried to render invalid page or invalid resolution" << page;
    return nullptr;
  }
  QImage image = docpage->renderToImage(72. * resolution, 72. * resolution);
  if (image.isNull()) {
    qWarning() << "Rendering page to image failed";
    return nullptr;
  }
  switch (page_part) {
    case LeftHalf:
      image = image.copy(0, 0, image.width() / 2, image.height());
      break;
    case RightHalf:
      image = image.copy((image.width() + 1) / 2, 0, image.width() / 2,
                         image.height());
      break;
    default:
      break;
  }
  QByteArray *const bytes = new QByteArray();
  QBuffer buffer(bytes);
  if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, "PNG")) {
    qWarning() << "Saving page as PNG image failed";
    delete bytes;
    return nullptr;
  }
  return new PngPixmap(bytes, page, resolution);
}

int PopplerDocument::overlaysShifted(const int start,
                                     const int shift_overlay) const
{
  // Get the "number" part of shift_overlay by removing the "overlay" flags.
  int shift = shift_overlay >= 0 ? shift_overlay & ~ShiftOverlays::AnyOverlay
                                 : shift_overlay | ShiftOverlays::AnyOverlay;
  // Check whether the document has non-trivial page labels and shift has
  // non-trivial overlay flags.
  if (pageLabels.empty() || shift == shift_overlay) return start + shift;
  // Find the beginning of next slide.
  QMap<int, QString>::const_iterator it = pageLabels.upperBound(start);
  // Shift the iterator according to shift.
  while (shift > 0 && it != pageLabels.cend()) {
    --shift;
    ++it;
  }
  while (shift < 0 && it != pageLabels.cbegin()) {
    ++shift;
    --it;
  }
  // Check if the iterator has reached the beginning or end of the set.
  if (it == pageLabels.cbegin()) return 0;
  if (it == pageLabels.cend()) {
    if (shift_overlay & FirstOverlay) return (--it).key();
    return doc->numPages() - 1;
  }
  // Return first or last overlay depending on overlay flags.
  if (shift_overlay & FirstOverlay) return (--it).key();
  return it.key() - 1;
}

void PopplerDocument::loadPageLabels()
{
  // Poppler functions for converting between labels and numbers seem to be
  // optimized for normal documents and are probably inefficient for handling
  // page numbers in presentations with overlays.
  // Use a lookup table.
  // This function for filling the lookup table is probably also inefficient.

  pageLabels.clear();
  if (!doc) return;

  // Check whether it makes sense to create the lookup table.
  QString label = "\n\n\n\n";
  for (int i = 0; i < doc->numPages(); i++) {
    const std::unique_ptr<Poppler::Page> page(doc->page(i));
    if (page && label != page->label()) {
      label = page->label();
      pageLabels.insert(i, label);
    }
  }
  if (pageLabels.firstKey() != 0) pageLabels[0] = "";

  // Add all pages explicitly to pageLabels, which have an own outline entry.
  const int num_pages = doc->numPages();
  QMap<int, QString>::key_iterator it;
  for (const auto &entry : std::as_const(outline)) {
    if (entry.page < 0 || entry.page >= num_pages) continue;
    it = std::upper_bound(pageLabels.keyBegin(), pageLabels.keyEnd(),
                          entry.page);
    if (it != pageLabels.keyBegin() && *--it != entry.page)
      pageLabels.insert(entry.page, pageLabels[*it]);
  }
}

const PdfLink *PopplerDocument::linkAt(const int page,
                                       const QPointF &position) const
{
  const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
  if (!docpage) return nullptr;
  const QSizeF pageSize = docpage->pageSizeF();
  const QPointF relpos = {position.x() / pageSize.width(),
                          position.y() / pageSize.height()};
  const auto links = docpage->links();
  for (auto it = links.cbegin(); it != links.cend(); ++it) {
    if ((*it)->linkArea().normalized().contains(relpos)) {
      QRectF rect = (*it)->linkArea().normalized();
      rect = {pageSize.width() * rect.x(), pageSize.height() * rect.y(),
              pageSize.width() * rect.width(),
              pageSize.height() * rect.height()};
      switch ((*it)->linkType()) {
        case Poppler::Link::Goto: {
          const Poppler::LinkGoto *gotolink =
#if (QT_VERSION_MAJOR >= 6)
              static_cast<Poppler::LinkGoto *>(it->get());
#else
              static_cast<Poppler::LinkGoto *>(*it);
#endif
          if (gotolink->isExternal())
            return new ExternalLink(PdfLink::ExternalPDF, rect,
                                    gotolink->fileName());
          else
            return new GotoLink(rect, gotolink->destination().pageNumber() - 1);
        }
        case Poppler::Link::Action: {
          const Poppler::LinkAction *actionlink =
#if (QT_VERSION_MAJOR >= 6)
              static_cast<Poppler::LinkAction *>(it->get());
#else
              static_cast<Poppler::LinkAction *>(*it);
#endif
          Action action = NoAction;
          switch (actionlink->actionType()) {
            case Poppler::LinkAction::PageFirst:
              action = FirstPage;
              break;
            case Poppler::LinkAction::PageLast:
              action = LastPage;
              break;
            case Poppler::LinkAction::PagePrev:
              action = PreviousPage;
              break;
            case Poppler::LinkAction::PageNext:
              action = NextPage;
              break;
            case Poppler::LinkAction::HistoryBack:
              // TODO: This will not work with notes and presentation combined
              // in same PDF.
              action = UndoDrawing;
              break;
            case Poppler::LinkAction::HistoryForward:
              // TODO: This will not work with notes and presentation combined
              // in same PDF.
              action = RedoDrawing;
              break;
            case Poppler::LinkAction::EndPresentation:
            case Poppler::LinkAction::Presentation:
              action = FullScreen;
              break;
            default:
              continue;
              /* For completeness: here are the other possible actions.
              // GoToPage does not make much sense when the destination is
              unknown. case Poppler::LinkAction::GoToPage:
              // Find is currently not implemented.
              case Poppler::LinkAction::Find:
              // Print will probably never be implemented.
              case Poppler::LinkAction::Print:
              // Quit and close are intentionally not handled to avoid
              unintended
              // closing of the program.
              case Poppler::LinkAction::Quit:
              case Poppler::LinkAction::Close:
                  continue;
              */
          }
          return new ActionLink(rect, action);
        }
        case Poppler::Link::Browse: {
          const Poppler::LinkBrowse *browselink =
#if (QT_VERSION_MAJOR >= 6)
              static_cast<Poppler::LinkBrowse *>(it->get());
#else
              static_cast<Poppler::LinkBrowse *>(*it);
#endif
          const QUrl url = preferences()->resolvePath(browselink->url());
          if (url.isValid())
            return new ExternalLink(
                url.isLocalFile() ? PdfLink::LocalUrl : PdfLink::RemoteUrl,
                rect, url);
          break;
        }
        case Poppler::Link::Movie: {
          const Poppler::LinkMovie *movielink =
#if (QT_VERSION_MAJOR >= 6)
              static_cast<Poppler::LinkMovie *>(it->get());
#else
              static_cast<Poppler::LinkMovie *>(*it);
#endif
          // TODO: currently this action is not connected to one specific movie
          // annotation.
          switch (movielink->operation()) {
            case Poppler::LinkMovie::Pause:
            case Poppler::LinkMovie::Stop:
              return new ActionLink(rect, PauseMedia);
            case Poppler::LinkMovie::Play:
            case Poppler::LinkMovie::Resume:
              return new ActionLink(rect, PlayMedia);
          }
          break;
        }
        case Poppler::Link::Sound: {
          const Poppler::LinkSound *soundlink =
#if (QT_VERSION_MAJOR >= 6)
              static_cast<Poppler::LinkSound *>(it->get());
#else
              static_cast<Poppler::LinkSound *>(*it);
#endif
          const Poppler::SoundObject *sound = soundlink->sound();
          switch (sound->soundType()) {
            case Poppler::SoundObject::Embedded:
              debug_verbose(DebugMedia,
                            "Found sound annotation: embedded on page" << page);
              if (!sound->data().isEmpty()) {
                std::shared_ptr<EmbeddedAudio> media(
                    embeddedSound(sound, rect));
                media->setVolume(soundlink->volume());
                media->setMode(soundlink->repeat() ? MediaAnnotation::Once
                                                   : MediaAnnotation::Repeat);
                return new MediaLink(PdfLink::SoundLink, rect, media);
              }
              break;
            case Poppler::SoundObject::External: {
              const QUrl url = preferences()->resolvePath(sound->url());
              debug_verbose(DebugMedia, "Found sound annotation:"
                                            << url << "on page" << page);
              if (url.isValid())
                return new MediaLink(
                    PdfLink::SoundLink, rect,
                    std::shared_ptr<MediaAnnotation>(
                        new ExternalMedia(url, rect, MediaAnnotation::Once)));
              break;
            }
          }
          break;
        }
        /* For completeness: These are the unhandled link types.
        case Poppler::Link::OCGState:
        case Poppler::Link::Execute:
        case Poppler::Link::Hide:
        case Poppler::Link::JavaScript:
        case Poppler::Link::Rendition:
        case Poppler::Link::None:
        */
        default:
          debug_msg(DebugRendering,
                    "Ignoring unsupported link" << (*it)->linkType());
      }
    }
  }
  return nullptr;
}

QList<std::shared_ptr<MediaAnnotation>> PopplerDocument::annotations(
    const int page)
{
  const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
  if (!docpage) return {};
  debug_verbose(DebugMedia, "Found" << docpage->annotations().size()
                                    << "annotations on page" << page);
  for (const auto &it : docpage->annotations())
    debug_verbose(DebugMedia, it->subType());
  const auto annotations = docpage->annotations(
      {Poppler::Annotation::AMovie, Poppler::Annotation::ASound,
       Poppler::Annotation::ARichMedia, Poppler::Annotation::AScreen});
  const auto links = docpage->links();
  debug_verbose(DebugMedia, "Found" << links.size() << "links on page" << page);
  if (annotations.empty() && links.empty()) return {};
  const QSizeF pageSize = docpage->pageSizeF();
  QList<std::shared_ptr<MediaAnnotation>> list;
  for (auto it = annotations.cbegin(); it != annotations.cend(); ++it) {
    debug_verbose(DebugMedia, "Checking annotation:" << (*it)->subType()
                                                     << "on page" << page);
    // TODO: try to find better way of handling URLs
    switch ((*it)->subType()) {
      case Poppler::Annotation::AMovie: {
        const Poppler::MovieObject *movie =
#if (QT_VERSION_MAJOR >= 6)
            static_cast<Poppler::MovieAnnotation *>(it->get())->movie();
#else
            static_cast<Poppler::MovieAnnotation *>(*it)->movie();
#endif
        const QUrl url = preferences()->resolvePath(movie->url());
        if (url.isValid()) {
          MediaAnnotation::Mode mode = MediaAnnotation::Once;
          switch (movie->playMode()) {
            case Poppler::MovieObject::PlayOpen:
              mode = MediaAnnotation::Open;
              break;
            case Poppler::MovieObject::PlayPalindrome:
              mode = MediaAnnotation::Palindrome;
              break;
            case Poppler::MovieObject::PlayRepeat:
              mode = MediaAnnotation::Repeat;
              break;
            default:
              break;
          }
          QRectF rect = (*it)->boundary();
          rect = {pageSize.width() * rect.x(), pageSize.height() * rect.y(),
                  pageSize.width() * rect.width(),
                  pageSize.height() * rect.height()};
          list.append(std::shared_ptr<MediaAnnotation>(
              new ExternalMedia(url, rect, mode)));
          debug_verbose(DebugMedia,
                        "Found video annotation:" << url << "on page" << page);
        }
        break;
      }
      case Poppler::Annotation::AScreen: {
#if (QT_VERSION_MAJOR >= 6)
        const auto sannotation =
            static_cast<Poppler::ScreenAnnotation *>(it->get());
#else
        const auto sannotation = static_cast<Poppler::ScreenAnnotation *>(*it);
#endif
        if (!sannotation->action()) break;
        Poppler::MediaRendition *rendition = sannotation->action()->rendition();
        if (!rendition) break;
        int flags = MediaAnnotation::HasAudio | MediaAnnotation::Interactive;
        if (rendition->autoPlay()) flags |= MediaAnnotation::Autoplay;
        if (rendition->showControls()) flags |= MediaAnnotation::ShowSlider;
        MediaAnnotation::Mode mode = MediaAnnotation::Once;
        if (rendition->repeatCount() > 1) mode = MediaAnnotation::Repeat;
        QRectF rect = sannotation->boundary();
        rect = {pageSize.width() * rect.x(), pageSize.height() * rect.y(),
                pageSize.width() * rect.width(),
                pageSize.height() * rect.height()};
        const QStringList type = rendition->contentType().split("/");
        if (type.first() == "video") {
          flags |= MediaAnnotation::HasVideo;
          if (rendition->isEmbedded()) {
            debug_verbose(
                DebugMedia,
                "Found embedded video (screen) annotation: on page" << page);
            std::shared_ptr<QByteArray> data =
                std::make_shared<QByteArray>(rendition->data());
            list.append(std::shared_ptr<MediaAnnotation>(
                new EmbeddedMedia(data, rect, mode, flags)));
          } else {
            debug_verbose(
                DebugMedia,
                "Found external video (screen) annotation: on page" << page);
            list.append(std::shared_ptr<MediaAnnotation>(
                new ExternalMedia(rendition->fileName(), rect, mode, flags)));
          }
        } else if (type.first() == "audio") {
          // TODO (is this actually relevant?)
          debug_verbose(
              DebugMedia,
              "Found unsupported audio (screen) annotation: on page" << page);
        }
        break;
      }
      case Poppler::Annotation::ASound: {
        const Poppler::SoundObject *sound =
#if (QT_VERSION_MAJOR >= 6)
            static_cast<Poppler::SoundAnnotation *>(it->get())->sound();
#else
            static_cast<Poppler::SoundAnnotation *>(*it)->sound();
#endif
        QRectF rect = (*it)->boundary();
        rect = {pageSize.width() * rect.x(), pageSize.height() * rect.y(),
                pageSize.width() * rect.width(),
                pageSize.height() * rect.height()};
        switch (sound->soundType()) {
          case Poppler::SoundObject::Embedded:
            debug_verbose(DebugMedia,
                          "Found sound annotation: embedded on page" << page);
            if (!sound->data().isEmpty())
              list.append(
                  std::shared_ptr<MediaAnnotation>(embeddedSound(sound, rect)));
            break;
          case Poppler::SoundObject::External: {
            const QUrl url = preferences()->resolvePath(sound->url());
            debug_verbose(DebugMedia, "Found sound annotation:"
                                          << url << "on page" << page);
            if (url.isValid())
              list.append(std::shared_ptr<MediaAnnotation>(
                  new ExternalMedia(url, rect, MediaAnnotation::Once)));
            break;
          }
        }
        break;
      }
      case Poppler::Annotation::ARichMedia:
        qWarning() << "Unsupported media type: rich media";
        break;
      default:
        break;
    }
  }
  for (auto it = links.cbegin(); it != links.cend(); ++it) {
    debug_verbose(DebugMedia, "Link of type" << (*it)->linkType()
                                             << (*it)->linkArea() << page);
    if ((*it)->linkType() == Poppler::Link::Sound) {
#if (QT_VERSION_MAJOR >= 6)
      const Poppler::LinkSound *link =
          static_cast<Poppler::LinkSound *>(it->get());
#else
      const Poppler::LinkSound *link = static_cast<Poppler::LinkSound *>(*it);
#endif
      QRectF rect = link->linkArea();
      rect = {pageSize.width() * rect.x(), pageSize.height() * rect.y(),
              pageSize.width() * rect.width(),
              pageSize.height() * rect.height()};
      switch (link->sound()->soundType()) {
        case Poppler::SoundObject::Embedded:
          debug_verbose(DebugMedia,
                        "Found sound link: embedded on page" << page);
          if (!link->sound()->data().isEmpty()) {
            auto media = std::shared_ptr<EmbeddedAudio>(
                embeddedSound(link->sound(), rect));
            media->setVolume(link->volume());
            media->setMode(link->repeat() ? MediaAnnotation::Once
                                          : MediaAnnotation::Repeat);
            list.append(media);
          }
          break;
        case Poppler::SoundObject::External: {
          const QUrl url = preferences()->resolvePath(link->sound()->url());
          debug_verbose(DebugMedia,
                        "Found sound link:" << url << "on page" << page);
          if (url.isValid())
            list.append(std::shared_ptr<MediaAnnotation>(
                new ExternalMedia(url, rect, MediaAnnotation::Once)));
          break;
        }
      }
    }
  }
  return list;
}

const SlideTransition PopplerDocument::transition(const int page) const
{
  const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
  if (!docpage) return SlideTransition();
  const Poppler::PageTransition *doc_trans = docpage->transition();
  if (!doc_trans) return SlideTransition();

  SlideTransition trans;
  trans.type = static_cast<SlideTransition::Type>(doc_trans->type());
  trans.duration = doc_trans->durationReal();
  trans.properties =
      (doc_trans->alignment() == Poppler::PageTransition::Vertical
           ? SlideTransition::Vertical
           : 0) |
      (doc_trans->direction() == Poppler::PageTransition::Outward
           ? SlideTransition::Outwards
           : 0);
  trans.angle = doc_trans->angle();
  if (trans.type == SlideTransition::Fly) {
    if (doc_trans->isRectangular()) trans.type = SlideTransition::FlyRectangle;
    trans.scale = doc_trans->scale();
  }
  return trans;
}

bool PopplerDocument::flexiblePageSizes() noexcept
{
  if (flexible_page_sizes >= 0 || doc == nullptr) return flexible_page_sizes;
  const QSizeF ref_size = doc->page(0)->pageSizeF();
  for (int page = 1; page < doc->numPages(); page++) {
    const std::unique_ptr<Poppler::Page> docpage(doc->page(page));
    if (docpage && docpage->pageSizeF() != ref_size) {
      flexible_page_sizes = 1;
      return 1;
    }
  }
  flexible_page_sizes = 0;
  return 0;
}

void PopplerDocument::loadOutline()
{
  // TODO: a huge outline will probably lead to a crash of the program.
  outline.clear();
  outline.append({"", -1, 1});
  const QVector<Poppler::OutlineItem> root = doc->outline();
  if (root.isEmpty()) return;
  // dangerous anonymous recursion
  auto fill_outline = [&](const Poppler::OutlineItem &entry, const char sign,
                          auto &function) -> void {
    const int idx = outline.length();
    outline.append({entry.name(), entry.destination()->pageNumber() - 1, -1});
    const auto &children = entry.children();
    if (!children.isEmpty()) {
      const auto end = children.cend() - 1;
      auto it = children.cbegin();
      while (it != end) function(*it++, 1, function);
      function(*it, -1, function);
    }
    outline[idx].next = sign * outline.length();
  };
  const auto end = root.cend() - 1;
  auto it = root.cbegin();
  while (it != end) fill_outline(*it++, 1, fill_outline);
  fill_outline(*it, -1, fill_outline);
#ifdef QT_DEBUG
  if ((preferences()->debug_level & (DebugRendering | DebugVerbose)) ==
      (DebugRendering | DebugVerbose))
    for (int i = 0; i < outline.length(); i++)
      qDebug() << DebugRendering << i << outline[i].page << outline[i].next
               << outline[i].title;
#endif
}

std::pair<int, QList<QRectF>> PopplerDocument::searchAll(const QString &needle,
                                                         int start_page,
                                                         bool forward) const
{
  if (needle.isEmpty() || !doc) return {-1, {}};
  if (start_page < 0)
    start_page = 0;
  else if (start_page >= doc->numPages())
    start_page = doc->numPages() - 1;
  QList<QRectF> results;
  if (forward)
    for (int page = start_page; page < doc->numPages(); ++page) {
      results = doc->page(page)->search(
          needle, Poppler::Page::IgnoreCase | Poppler::Page::IgnoreDiacritics);
      if (!results.isEmpty()) return {page, results};
    }
  else
    for (int page = start_page; page >= 0; --page) {
      results = doc->page(page)->search(
          needle, Poppler::Page::IgnoreCase | Poppler::Page::IgnoreDiacritics);
      if (!results.isEmpty()) return {page, results};
    }
  return {-1, {}};
}
