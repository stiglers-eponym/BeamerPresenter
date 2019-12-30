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
#include <poppler-version.h>
#include <QDomDocument>
#include <QInputDialog>
#include "enumerates.h"

class PdfDoc
{
private:
    Poppler::Document* popplerDoc = nullptr;
    QString pdfPath;
    QList<Poppler::Page*> pdfPages;
    QList<QString> labels;
    PagePart pagePart = PagePart::FullPage;
    QDateTime lastModified = QDateTime();

public:
    PdfDoc(QString pathToPdf) : pdfPath(pathToPdf) {}
    ~PdfDoc();
    bool loadDocument();

    Poppler::Document const * getDoc() const {return popplerDoc;}
    QList<Poppler::Page*> const* getPages() const {return &pdfPages;}
    Poppler::Page* getPage(int pageNumber) const;
    QDomDocument const * getToc() const {return popplerDoc->toc();}
    QSize getPageSize(int const pageNumber) const;
    QString const& getLabel(int const pageNumber) const;
    int getNextSlideIndex(int const index) const;
    int getPreviousSlideEnd(int const index) const;
    int getSlideNumber(int const page) const {return pdfPages[page]->label().toInt();}
    int destToSlide(QString const & dest) const;
    QString const& getPath() const {return pdfPath;}

    void setPagePart(PagePart const state) {pagePart = state;}
};

#endif // PDFWIDGET_H
