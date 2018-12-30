/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "pdfdoc.h"

PdfDoc::PdfDoc(QString pathToPdf)
{
    pdfPath = pathToPdf;
}

PdfDoc::~PdfDoc()
{
    pdfPages.clear();
    delete popplerDoc;
}

void PdfDoc::loadDocument()
{
    popplerDoc = Poppler::Document::load(pdfPath);
    popplerDoc->setRenderHint( Poppler::Document::Antialiasing );
    popplerDoc->setRenderHint( Poppler::Document::TextAntialiasing );
    popplerDoc->setRenderHint( Poppler::Document::TextHinting );

    for ( int i=0; i < popplerDoc->numPages(); i++ ) {
        Poppler::Page * p = popplerDoc->page(i);
        pdfPages.append( p );
    }
}

QSize PdfDoc::getPageSize(int const pageNumber) const
{
    if (pageNumber < 0)
        return pdfPages.at(0)->pageSize();
    if (pageNumber >= popplerDoc->numPages())
        return pdfPages.at(popplerDoc->numPages()-1)->pageSize();
    return pdfPages.at(pageNumber)->pageSize();
}

Poppler::Page* PdfDoc::getPage(int pageNumber) const
{
    if (pageNumber < 0)
        return pdfPages.at(0);
    if (pageNumber >= popplerDoc->numPages())
        return pdfPages.at(popplerDoc->numPages()-1);
    return pdfPages.at(pageNumber);
}
