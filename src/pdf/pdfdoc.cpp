/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pdfdoc.h"

PdfDoc::~PdfDoc()
{
    qDeleteAll(pdfPages);
    pdfPages.clear();
    delete popplerDoc;
}

bool PdfDoc::loadDocument()
{
    // (Re)load the pdf document.
    // Return true if a new document has been loaded and false otherwise.
    // If a document has been loaded before, this function checks whether the file has been updated and reloads it if necessary.

    // Check if the file is valid and readable and save its last modification time
    QFileInfo file = QFileInfo(pdfPath);
    if (!file.exists() || !file.isFile()) {
        qCritical() << "File does not exist or is not a file:" << pdfPath;
        return false;
    }
    else if (!file.isReadable()) {
        qCritical() << "File is unreadable:" << pdfPath;
        return false;
    }
    if (file.suffix().toLower() != "pdf")
        qWarning() << "Interpreting the following file as PDF:" << pdfPath;

    // Check whether the file has been updated
    if (popplerDoc != nullptr && QFileInfo(pdfPath).lastModified() <= lastModified)
        return false;

    // Load the file
    Poppler::Document* newDoc = Poppler::Document::load(pdfPath);
    if (newDoc == nullptr) {
        qCritical() << "Failed to open document";
        return false;
    }
    // PDF files can be locked.
    // Using locked pdf files is untested.
    if (newDoc->isLocked()) {
        bool ok;
        QString userPassword = QInputDialog::getText(nullptr, "User password for " + pdfPath, "Using locked PDF files is untested!\nUser password:", QLineEdit::Password, "", &ok);
        if (!ok) {
            qCritical() << "No user password provided for locked document";
            delete newDoc;
            return false;
        }
        QString ownerPassword = QInputDialog::getText(nullptr, "Owner password for " + pdfPath, "Using locked PDF files is untested!\nOwner password:", QLineEdit::Password, "", &ok);
        if (!newDoc->unlock(QByteArray::fromStdString(ownerPassword.toStdString()), QByteArray::fromStdString(userPassword.toStdString()))) {
            qCritical() << "Failed to unlock document";
            delete newDoc;
            return false;
        }
    }

    // Set rendering hints
    newDoc->setRenderHint(Poppler::Document::TextAntialiasing);
    newDoc->setRenderHint(Poppler::Document::TextHinting);
    newDoc->setRenderHint(Poppler::Document::TextSlightHinting);
    newDoc->setRenderHint(Poppler::Document::Antialiasing);
    newDoc->setRenderHint(Poppler::Document::ThinLineShape);
#ifdef POPPLER_VERSION_MAJOR
#ifdef POPPLER_VERSION_MINOR
#if POPPLER_VERSION_MAJOR > 0 or POPPLER_VERSION_MINOR >= 60
    newDoc->setRenderHint(Poppler::Document::HideAnnotations);
#endif
#endif
#endif

    // Clear old lists
    qDeleteAll(pdfPages);
    pdfPages.clear();
    labels.clear();

    // Create lists of pages.
    for (int i=0; i < newDoc->numPages(); i++) {
        Poppler::Page* p = newDoc->page(i);
        pdfPages.append(p);
        labels.append(p->label());
    }

    // Check document contents and print warnings if unimplemented features are found.
    if (newDoc->hasOptionalContent())
        qWarning() << "This file has optional content. Optional content is not supported.";
    if (newDoc->hasEmbeddedFiles())
        qWarning() << "This file contains embedded files. Embedded files are not supported.";
    if (newDoc->scripts().size() != 0)
        qWarning() << "This file contains JavaScript scripts. JavaScript is not supported.";

    // Delete the old document and replace it by the new document.
    delete popplerDoc;
    popplerDoc = newDoc;
    lastModified = file.lastModified();
    return true;
}

QSizeF const PdfDoc::getPageSize(int const pageNumber) const
{
    // Return page size in point = inch/72
    if (pageNumber < 0)
        return pdfPages[0]->pageSizeF();
    if (pageNumber >= popplerDoc->numPages())
        return pdfPages[popplerDoc->numPages()-1]->pageSizeF();
    return pdfPages[pageNumber]->pageSizeF();
}

Poppler::Page const* PdfDoc::getPage(int pageNumber) const
{
    // Check if page number is valid and return page.
    if (pageNumber < 0)
        return pdfPages[0];
    if (pageNumber >= popplerDoc->numPages())
        return pdfPages[popplerDoc->numPages()-1];
    return pdfPages[pageNumber];
}

int PdfDoc::getNextSlideIndex(int const index) const
{
    // Return the index of the next slide, which is not just an overlay of the current slide.
    QString label = pdfPages[index]->label();
    for (int i=index; i<popplerDoc->numPages(); i++) {
        if (label != pdfPages[i]->label())
            return i;
    }
    return popplerDoc->numPages()-1;
}

int PdfDoc::getPreviousSlideEnd(int const index) const
{
    // Return the index of the last overlay of the previous slide.
    QString label = pdfPages[index]->label();
    for (int i=index; i>=0; i--) {
        if (label != pdfPages[i]->label()) {
            // Get the duration. Avoid returning a page of duration of less than one second.
            double duration = pdfPages[i]->duration();
            int j = i;
            // Don't return the index of a slides which is shown for less than one second.
            while (duration > -0.01 && duration < 1. && j > 0 && pdfPages[j]->label() == pdfPages[i]->label())
                duration = pdfPages[--j]->duration();
            return j;
        }
    }
    return 0;
}

int PdfDoc::destToSlide(QString const & dest) const
{
    // Return the index of the page, which is bookmarked as dest in the pdf.
    Poppler::LinkDestination* linkDest = popplerDoc->linkDestination(dest);
    // Return -1 if an invalid destination is given.
    if (linkDest == nullptr)
        return -1;
    // page = page index = page number - 1
    int const page = linkDest->pageNumber() - 1;
    delete linkDest;
    return page;
}

QString const PdfDoc::getLabel(int const pageNumber) const
{
    // Check whether pageNumber is valid. Return its label.
    if (pageNumber < 0)
        return pdfPages.first()->label();
    else if (pageNumber < popplerDoc->numPages())
        return pdfPages[pageNumber]->label();
    else
        return pdfPages.last()->label();
}

int PdfDoc::getSlideNumber(const int page) const
{
    // Check whether page has a label.
    QString label = pdfPages[page]->label();
    if (label.isEmpty())
        return page + 1;
    else
        return label.toInt();
}

Poppler::Page const* PdfDoc::getPage(QString const& pageLabel) const
{
    int const idx = labels.indexOf(pageLabel);
    if (idx > 0)
        return pdfPages[idx];
    return nullptr;
}
