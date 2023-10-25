// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/config.h"
#include "src/rendering/pdfdocument.h"
#ifdef USE_EXTERNAL_RENDERER
#include "src/rendering/externalrenderer.h"
#endif
#include "src/log.h"
#include "src/preferences.h"
#include "src/rendering/pixcachethread.h"
#include "src/rendering/pngpixmap.h"

void PixCacheThread::setNextPage(const PixCacheThread *target,
                                 const int page_number, const qreal res)
{
  if (target == this && !isRunning()) {
    page = page_number;
    resolution = res;
    start(QThread::LowPriority);
  }
}

void PixCacheThread::run()
{
  // Check if a renderer is available.
  if (renderer == nullptr || resolution <= 0. || page < 0) return;

  // Render the image. This is takes some time.
  debug_msg(DebugCache,
            "Rendering in cache thread:" << page << resolution << this);
  const PngPixmap *image = renderer->renderPng(page, resolution);

  // Send the image to pixcache master.
  if (image) emit sendData(image);
}

bool PixCacheThread::initializeRenderer(const PdfDocument *const doc,
                                        const PagePart page_part)
{
  // Create the renderer without any checks.
#ifdef USE_EXTERNAL_RENDERER
  if (preferences()->renderer == renderer::ExternalRenderer)
    renderer = new ExternalRenderer(preferences()->rendering_command,
                                    preferences()->rendering_arguments, doc,
                                    page_part);
  else
#endif
    renderer = doc->createRenderer(page_part);

  // Check if the renderer was created successfully.
  if (renderer->isValid()) return true;

  // Creating renderer failed. Clean up and return false.
  qCritical() << tr("Creating renderer failed") << preferences()->renderer;
  delete renderer;
  renderer = nullptr;
  return false;
}
