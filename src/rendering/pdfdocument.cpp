#include "src/rendering/pdfdocument.h"

bool operator<(const int page, const PdfOutlineEntry& other)
{return page < other.page;}

const PdfOutlineEntry &PdfDocument::outlineEntryAt(const int page) const
{
    const auto it = std::upper_bound(outline.cbegin(), outline.cend(), page);
    return it == outline.cbegin() ? *it : *(it-1);
}
