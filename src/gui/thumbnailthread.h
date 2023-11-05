// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef THUMBNAILTHREAD_H
#define THUMBNAILTHREAD_H

#include <QList>
#include <QObject>
#include <memory>

#include "src/config.h"
#include "src/rendering/abstractrenderer.h"

class QPixmap;
class PdfDocument;

/**
 * @brief Worker object for rendering thumbnails in own thread
 *
 * Created by ThumbnailWidget and moved to own thread, the ThumbnailThread
 * object renders thumbnail images and sends them to ThumbailWidget.
 * The images are connected to the ThumbnailButtons, at which they will
 * be shown.
 *
 * The images are not directly shown in the buttons from this thread,
 * because that should happen in the main thread.
 *
 * @see ThumbnailWidget
 * @see ThumbnailButton
 */
class ThumbnailThread : public QObject
{
  Q_OBJECT

  /// container of page, button and resolution as queued for rendering
  struct queue_entry {
    /// Button which should receive the thumbnail
    int button_index;
    /// thumbnail resolution
    qreal resolution;
    /// page index
    int page;
  };

  /// renderer, owned by this, created in constructor.
  AbstractRenderer *renderer{nullptr};
  /// document, not owned by this.
  std::shared_ptr<const PdfDocument> document;
  /// queue of pages/thumbnails which should be rendered
  QList<queue_entry> queue;

 protected:
  /// Timer event: render next slide;
  virtual void timerEvent(QTimerEvent *event);

 public:
  /// Constructor: create renderer if document is not nullptr.
  ThumbnailThread(std::shared_ptr<const PdfDocument> document = nullptr);

  /// Destructor: delete renderer.
  ~ThumbnailThread() { delete renderer; }

 public slots:
  /// Add entries to rendering queue.
  void append(int button_index, qreal resolution, int page)
  {
    if (resolution > 0) queue.append({button_index, resolution, page});
  }

  /// Clear the queue.
  void clearQueue() { queue.clear(); }

  /// Do the work: render thumbnails for the queued pages.
  void renderImages() { startTimer(0); }

 signals:
  /// Send thumbnail back to ThumbnailWidget, which sets the pixmap
  /// from the main thread.
  void sendThumbnail(int button_index, const QPixmap pixmap);
};

#endif  // THUMBNAILTHREAD_H
