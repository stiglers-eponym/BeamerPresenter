// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef PIXCACHETHREAD_H
#define PIXCACHETHREAD_H

#include <QThread>
#include "src/config.h"
#include "src/enumerates.h"

class PngPixmap;
class PdfDocument;
class AbstractRenderer;

/**
 * @brief Separate thread for rendering page pixmaps to (compressed) cache.
 */
class PixCacheThread : public QThread
{
    Q_OBJECT

private:
    /// Renderer doing the main work.
    AbstractRenderer *renderer {nullptr};

    /// resolution in pixels per point (dpi/72).
    qreal resolution;

    /// page number (index)
    int page;

public:
    /// Constructor: initialize thread and renderer.
    PixCacheThread(const PdfDocument * const doc, const PagePart page_part = FullPage, QObject *parent = nullptr);

    /// Destructor: delete renderer.
    ~PixCacheThread();

    /// Create a renderer based on preferences.
    /// Return true if successful and false if no renderer was created.
    bool initializeRenderer(const PdfDocument * const doc, const PagePart page_part = FullPage);

    /// Do the work: renderer to actually renders the page. Emits sendData.
    void run() override;

    /// Set page number and resolution.
    /// Note that this is not thread save! But when starting to run the thread,
    /// the values are locally copied. That should make it nearly impossible to
    /// create problems using this function.
    void setNextPage(const int page_number, const qreal res);

signals:
    /// Send out the data.
    void sendData(const PngPixmap *data);
};

#endif // PIXCACHETHREAD_H
