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
    labels.clear();
    delete popplerDoc;
}

void PdfDoc::loadDocument()
{
    if (popplerDoc != nullptr) {
        std::cerr << "WARNING: A document has already been loaded. Deleting it in order to avoid memory leaks." << std::endl;
        delete popplerDoc;
    }
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

    qDeleteAll(pdfPages);
    pdfPages.clear();
    labels.clear();
    for ( int i=0; i < popplerDoc->numPages(); i++ ) {
        Poppler::Page * p = popplerDoc->page(i);
        pdfPages.append( p );
        labels.append( p->label() );
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

int PdfDoc::getNextSlideIndex(int const index) const
{
    QString label = labels.at(index);
    for (int i=index; i<popplerDoc->numPages(); i++) {
        if (label != labels.at(i))
            return i;
    }
    return popplerDoc->numPages()-1;
}

int PdfDoc::getPreviousSlideEnd(int const index) const
{
    QString label = labels.at(index);
    for (int i=index; i>0; i--) {
        if (label != labels.at(i)) {
            double duration = pdfPages.at(i)->duration();
            int j=i;
            // Don't return the index of a slides which is shown for less than one second
            while (duration > -0.01 && duration < 1. && (labels.at(j) == labels.at(i)))
                duration = pdfPages.at(--j)->duration();
            return j;
        }
    }
    return 0;
}

void PdfDoc::setPagePart(int const state)
{
    pagePart = state;
}
