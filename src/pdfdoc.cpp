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
    qDeleteAll(pdfPages);
    pdfPages.clear();
    delete popplerDoc;
}

void PdfDoc::loadDocument()
{
    popplerDoc = Poppler::Document::load(pdfPath);
    if (popplerDoc->isLocked()) {
        // TODO: use a nicer way of entering passwords (a QDialog?)
        std::cout << "WARNING: File " << pdfPath.toStdString() << ":\n"
                  << "This file is locked. Support for locked files is HIGHLY EXPERIMENTAL!" << std::endl
                  << "You can try to enter your password here.\n"
                  << "YOUR PASSWORD WILL BE VISIBLE IF YOU ENTER IT HERE!" << std::endl;
        std::string ownerPassword;
        std::string userPassword;
        std::cout << "Owner password (NOT HIDDEN!): ";
        std::cin >> ownerPassword;
        std::cout << "User password (NOT HIDDEN!): ";
        std::cin >> userPassword;
        popplerDoc->unlock(QByteArray::fromStdString(ownerPassword), QByteArray::fromStdString(userPassword));
    }
    popplerDoc->setRenderHint(Poppler::Document::TextAntialiasing);
    popplerDoc->setRenderHint(Poppler::Document::TextHinting);
    popplerDoc->setRenderHint(Poppler::Document::ThinLineShape);
    popplerDoc->setRenderHint(Poppler::Document::Antialiasing);

    for ( int i=0; i < popplerDoc->numPages(); i++ ) {
        Poppler::Page * p = popplerDoc->page(i);
        pdfPages.append( p );
    }

    if (popplerDoc->hasOptionalContent())
        std::cout << "This file has optional content. Optional content is not supported." << std::endl;
    if (popplerDoc->hasEmbeddedFiles())
        std::cout << "This file contains embedded files. Embedded files are not supported." << std::endl;
    if (popplerDoc->scripts().size() != 0)
        std::cout << "This file contains JavaScript scripts. JavaScript is not supported." << std::endl;
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
