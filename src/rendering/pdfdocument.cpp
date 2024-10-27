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
    case PdfEngine::MuPdf:
      return new MuPdfRenderer(doc, page_part);
#endif
#ifdef USE_POPPLER
    case PdfEngine::Poppler:
      return new PopplerRenderer(doc, page_part);
#endif
#ifdef USE_QTPDF
    case PdfEngine::QtPDF:
      return new QtRenderer(doc, page_part);
#endif
    default:
      return nullptr;
  }
}

int PdfDocument::pageIndex(const QString &label) const
{
  if (pageLabels.isEmpty()) return label.toInt() - 1;
  // This is slow (linear time):
  return pageLabels.key(label, -1);
}

QList<int> PdfDocument::overlayIndices() const noexcept
{
  return pageLabels.size() == numberOfPages() ? QList<int>()
                                              : pageLabels.keys();
}

int PdfDocument::overlaysShifted(const int start, PageShift shift_overlay) const
{
  // Check whether the document has non-trivial page labels and shift has
  // non-trivial overlay flags.
  if (pageLabels.empty() || shift_overlay.overlay == ShiftOverlays::NoOverlay)
    return start + shift_overlay.shift;
  // Find the beginning of next slide.
  QMap<int, QString>::const_iterator it = pageLabels.upperBound(start);
  // Shift the iterator according to shift.
  while (shift_overlay.shift > 0 && it != pageLabels.cend()) {
    --shift_overlay.shift;
    ++it;
  }
  while (shift_overlay.shift < 0 && it != pageLabels.cbegin()) {
    ++shift_overlay.shift;
    --it;
  }
  // Check if the iterator has reached the beginning or end of the set.
  if (it == pageLabels.cbegin()) return 0;
  if (it == pageLabels.cend()) {
    if (shift_overlay.overlay == ShiftOverlays::FirstOverlay)
      return (--it).key();
    else
      return numberOfPages() - 1;
  }
  // Return first or last overlay depending on overlay flags.
  if (shift_overlay.overlay == ShiftOverlays::FirstOverlay)
    return (--it).key();
  else
    return it.key() - 1;
}

const QString PdfDocument::pageLabel(const int page) const
{
  // Check if the page number is valid.
  if (page < 0 || page >= numberOfPages()) return "";

  if (pageLabels.isEmpty()) return QString::number(page + 1);
  return (--pageLabels.upperBound(page)).value();
}
