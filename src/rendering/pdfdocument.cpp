#include "src/rendering/pdfdocument.h"

/// Compare outline entries by their page.
bool operator<(const int page, const PdfDocument::PdfOutlineEntry& other)
{return page < other.page;}

const PdfDocument::PdfOutlineEntry &PdfDocument::outlineEntryAt(const int page) const
{
    // Upper bound will always point to the next outline entry
    // (or outline.cend() or outline.cbegin()).
    const auto it = std::upper_bound(outline.cbegin(), outline.cend(), page);
    return it == outline.cbegin() ? *it : *(it-1);
}
