// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QScrollArea>
#include <QSize>
#include <memory>

#include "src/config.h"
#include "src/enumerates.h"
#include "src/gui/thumbnailbutton.h"

class QShowEvent;
class QKeyEvent;
class QPixmap;
class PdfDocument;
class ThumbnailThread;

/**
 * @brief Widget showing thumbnail slides on grid layout in scroll area.
 *
 * @see ThumbnailButton
 * @see ThumbnailThread
 *
 * @todo don't change slide after touch scroll event
 */
class ThumbnailWidget : public QScrollArea
{
  Q_OBJECT

  /// inverse tolerance for widget size changes for recalculating buttons
  static constexpr int inverse_tolerance = 10;

  /// maximum waiting time for rendering (ms)
  static constexpr int max_render_time_ms = 2000;

 public:
  enum ThumbnailFlag {
    /// show one thumbnail per page label instead of per page
    SkipOverlays = 1 << 0,
  };
  Q_DECLARE_FLAGS(ThumbnailFlags, ThumbnailFlag);
  Q_FLAG(ThumbnailFlags);

 private:
  /// QObject for rendering. which is moved to an own thread.
  /// Communication to render_thread is almost exclusively done via the
  /// signal/slot mechanism since it lives in another thread.
  ThumbnailThread *render_thread{nullptr};
  /// Document shown by these thumbnails.
  std::shared_ptr<const PdfDocument> document;

  /// width of widget when thumbnails were rendered, in pixels.
  int ref_width{0};
  /// number of columns
  unsigned char columns{4};
  /// flags: currently only SkipOverlays.
  ThumbnailFlags _flags = {};
  /// currently focused page index
  ThumbnailButton *focused_button{nullptr};
  /// button for current page
  ThumbnailButton *current_page_button{nullptr};

  /// Create widget and layout.
  void initialize();

  /// Initialize (create and start) rendering thread.
  void initRenderingThread();

  /// Create or replace thumbnail button.
  /// Ask render thread to render page preview.
  void createButton(const int display_page, const int link_page,
                    const int position, const int col_width);

 protected:
  /// Resize: clear if necessary.
  void resizeEvent(QResizeEvent *) override;

 public:
  /// Nearly trivial constructor.
  explicit ThumbnailWidget(
      std::shared_ptr<const PdfDocument> document = nullptr,
      QWidget *parent = nullptr);

  /// Destructor, stop and delete render thread.
  ~ThumbnailWidget();

  /// Set number of columns.
  void setColumns(const unsigned char n_columns) noexcept
  {
    columns = n_columns;
  }

  /// get function for _flags
  ThumbnailFlags &flags() noexcept { return _flags; }

  /// (re)generate and show the thumbnails. This initializes render_thread
  /// if it does not exist yet. By default it takes document from
  /// preferences(). The function returns after (creating,) instructing
  /// and starting the rendering thread. Rendering is then done in
  /// background.
  void generate();

  /// Preferred height depends on width.
  bool hasHeightForWidth() const noexcept override { return true; }

  /// Size hint for layout.
  QSize sizeHint() const noexcept override { return {100, 200}; }

  ThumbnailButton *buttonAtPage(int page);

 public slots:
  /// Set focus to given page.
  void focusPage(int page);

  /// generate thumbnails if necessary and select currenlty visible page.
  void showEvent(QShowEvent *event) override;

  /// Override key press events: Send page up and page down to master.
  void keyPressEvent(QKeyEvent *event) override;

  /// Receive thumbnail from render_thread and show it on button.
  void receiveThumbnail(int button_index, const QPixmap pixmap);

  /// Handle actions: clear if files are reloaded.
  void handleAction(const Action action);

  /// Remove focus from old button and focus this button.
  void setFocusButton(ThumbnailButton *button);

  /// Move focus to row above/below (updown=-1/+1)
  void moveFocusUpDown(const qint8 updown);

 signals:
  /// Tell render_thread to render page with resolution and associate it
  /// with given button index.
  void sendToRenderThread(int button_index, qreal resolution, int page);
  /// Tell render_thread to start rendering.
  void startRendering();
  /// Tell thumbnail thread to clear queue.
  void interruptThread();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ThumbnailWidget::ThumbnailFlags);

#endif  // THUMBNAILWIDGET_H
