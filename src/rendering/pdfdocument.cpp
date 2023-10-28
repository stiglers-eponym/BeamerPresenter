// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/pdfdocument.h"

#include "src/enumerates.h"
#ifdef USE_MUPDF
#include "src/rendering/mupdfrenderer.h"
#endif
#ifdef USE_POPPLER
#include "src/rendering/popplerrenderer.h"
#endif
#ifdef USE_QTPDF
#include "src/rendering/qtrenderer.h"
#endif

AbstractRenderer *createRenderer(const std::shared_ptr<const PdfDocument> &doc,
                                 const PagePart page_part)
{
  switch (doc->type()) {
#ifdef USE_MUPDF
    case MuPdfEngine:
      return new MuPdfRenderer(doc, page_part);
#endif
#ifdef USE_POPPLER
    case PopplerEngine:
      return new PopplerRenderer(doc, page_part);
#endif
#ifdef USE_QTPDF
    case QtPDFEngine:
      return new QtRenderer(doc, page_part);
#endif
    default:
      return nullptr;
  }
}
