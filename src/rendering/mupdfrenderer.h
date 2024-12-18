// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: AGPL-3.0-or-later

#ifndef MUPDFRENDERER_H
#define MUPDFRENDERER_H

#include <memory>

#include "src/config.h"
// old versions of MuPDF don't have 'extern "C"' in the header files.
extern "C" {
#include <mupdf/fitz.h>
}
#include "src/enumerates.h"
#include "src/rendering/abstractrenderer.h"
#include "src/rendering/mupdfdocument.h"

class QPixmap;
class PngPixmap;

/**
 * @brief MuPDF implementation of AbstractRenderer.
 *
 * Renderer using the MuPDF engine. This renderer requires that the PDF
 * document has been loaded by MuPDF (and not Poppler).
 * The rendering requires preparation done by const functions in the
 * MuPdfDocument (which in general lives in a differen thread).
 */
class MuPdfRenderer : public AbstractRenderer
{
  /// Document used for rendering. doc is not owned by this.
  const std::shared_ptr<const MuPdfDocument> doc;

  /// Helper function for rendering functions.
  fz_pixmap *renderFzPixmap(const int page, const qreal resolution,
                            fz_context *&ctx) const;

 public:
  /// Constructor: only initializes doc and page_part.
  MuPdfRenderer(const std::shared_ptr<const PdfDocument> &document,
                const PagePart part = FullPage)
      : AbstractRenderer(part),
        doc(document && document->type() == PdfEngine::MuPdf
                ? std::dynamic_pointer_cast<const MuPdfDocument>(document)
                : nullptr)
  {
  }

  /// Trivial destructor.
  ~MuPdfRenderer() override {}

  /// Render page to a QPixmap. Resolution is given in pixels per point
  /// (dpi/72).
  const QPixmap renderPixmap(const int page,
                             const qreal resolution) const override;

  /// Render page to PNG image stored in a QByteArray as part of a PngPixmap.
  /// Resolution is given in pixels per point (dpi/72).
  const PngPixmap *renderPng(const int page,
                             const qreal resolution) const override;

  /// In the current implementation this is always valid.
  bool isValid() const override { return doc && doc->isValid(); }
};

#endif  // MUPDFRENDERER_H
