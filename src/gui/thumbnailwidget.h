// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QScrollArea>
#include <QSize>
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
 * @todo clear when resizing?
 */
class ThumbnailWidget : public QScrollArea
{
    Q_OBJECT

public:
    enum
    {
        /// show one thumbnail per page label instead of per page
        SkipOverlays = 1 << 0,
    };

private:
    /// QObject for rendering. which is moved to an own thread.
    /// Communication to render_thread is almost exclusively done via the
    /// signal/slot mechanism since it lives in another thread.
    ThumbnailThread *render_thread = nullptr;

    /// width of widget when thumbnails were rendered, in pixels.
    int ref_width {0};
    /// number of columns
    unsigned char columns {4};
    /// flags: currently only SkipOverlays.
    unsigned char _flags {0};
    /// currently focussed page index
    int focussed_index {0};

    /// Set focus to given page.
    void focusPage(int page);

public:
    /// Nearly trivial constructor.
    explicit ThumbnailWidget(QWidget *parent = nullptr) : QScrollArea(parent)
    {setFocusPolicy(Qt::NoFocus);}

    /// Destructor, stop and delete render thread.
    ~ThumbnailWidget();

    /// Set number of columns.
    void setColumns(const unsigned char n_columns) noexcept
    {columns = n_columns;}

    /// get function for _flags
    unsigned char &flags() noexcept
    {return _flags;}

    /// (re)generate and show the thumbnails. This initializes render_thread
    /// if it does not exist yet. By default it takes document from
    /// preferences(). The function returns after (creating,) instructing
    /// and starting the rendering thread. Rendering is then done in
    /// background.
    void generate(const PdfDocument *document = NULL);

    /// Preferred height depends on width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

    /// Size hint for layout.
    QSize sizeHint() const noexcept override
    {return {100, 200};}

public slots:
    /// generate thumbnails if necessary and select currenlty visible page.
    void showEvent(QShowEvent *event) override;

    /// Override key press events: Send page up and page down to master.
    void keyPressEvent(QKeyEvent *event) override;

    /// Receive thumbnail from render_thread and show it on button.
    void receiveThumbnail(ThumbnailButton *button, const QPixmap pixmap)
    {if (button) button->setPixmap(pixmap);}

    /// Handle actions: clear if files are reloaded.
    void handleAction(const Action action);

    /// Remove focus from old button and focus this button.
    void setFocusIndex(const ThumbnailButton *button);

signals:
    /// Tell render_thread to render page with resolution and associate it
    /// with button.
    void sendToRenderThread(ThumbnailButton *button, qreal resolution, int page);
    /// Tell render_thread to start rendering.
    void startRendering();
};

#endif // THUMBNAILWIDGET_H
