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

#ifndef PDFWIDGET_H
#define PDFWIDGET_H

#include <QtDebug>
#include <iostream>
#include <QFileInfo>
#include <poppler-qt5.h>
#include <QDomDocument>
#include <QInputDialog>
#include "../enumerates.h"

#if __has_include(<poppler-version.h>)
// Not available in poppler <= 0.62.0:
#include <poppler-version.h>
#endif
// If the above inclusion fails on you system, you can comment it out.
// If no poppler version is defined, the compiler will assume that you use an old version (<0.60.0).
// To avoid this, you can manually define your poppler version here:
//#define POPPLER_VERSION "?.??.?" // only for the BeamerPresenter version string.
//#define POPPLER_VERSION_MAJOR 0
//#define POPPLER_VERSION_MINOR ??
//#define POPPLER_VERSION_MICRO ? // not needed


/// PDF document.
/// This provides an interface for caching all Poppler::Page objects and reloading files.
class PdfDoc
{
private:
    /// Poppler PDF document or nullptr if no document is loaded.
    Poppler::Document* popplerDoc = nullptr;
    /// Path to PDF file.
    QString pdfPath;
    /// List of all PDF pages.
    QList<Poppler::Page*> pdfPages;
    /// Last time of modification of the file in the form which was last loaded.
    /// This is used to check whether it needs to be reloaded.
    QDateTime lastModified = QDateTime();
    /// List of labels
    QList<QString> labels;

public:
    /// Constructor: takes the path to the PDF file as argument. This does not load the document.
    PdfDoc(QString const& pathToPdf) : pdfPath(pathToPdf) {}
    ~PdfDoc();
    /// Load the document. Returns true if the document was loaded successfully and false otherwise.
    bool loadDocument();

    /// Return a pointer to the PDF document.
    Poppler::Document const* getDoc() const {return popplerDoc;}
    /// Return a pointer to a constant list of all pages.
    QList<Poppler::Page*> const* getPages() const {return &pdfPages;}
    /// Check if page number is valid and return page.
    Poppler::Page const* getPage(int pageNumber) const;
    /// Check if page label is valid and return page.
    Poppler::Page const* getPage(QString const& pageLabel) const;
    /// Modification date as string.
    QDateTime const& getLastModified() const {return lastModified;}
    /// Return the QDomDocument representing the table of contents (TOC) of the PDF document.
    QDomDocument const* getToc() const {return popplerDoc->toc();}
    /// Return page size in point = inch/72.
    QSizeF const getPageSize(int const pageNumber) const;
    /// Return label of given page.
    QString const& getLabel(int const pageNumber) const;
    /// Return page index (number) of the next page with different page label.
    int getNextSlideIndex(int const index) const;
    /// Return page index (number) of the previous page with different page label.
    /// This function skips slides which have a duration of less than one second.
    int getPreviousSlideEnd(int const index) const;
    /// Return page index (number) of given page label.
    int getPageNumber(QString const& label) const {return labels.indexOf(label);}
    /// Return page index (number) of a destination string (from table of contents).
    /// Return -1 if an invalid destination string is given.
    int destToSlide(QString const& dest) const;
    /// Return the path to the PDF file.
    QString const& getPath() const {return pdfPath;}
};

#endif // PDFWIDGET_H
