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

#include <iostream>

#include <QWidget>
#include <QLabel>
#include <poppler-qt5.h>

class PdfDoc : public QObject
{
    Q_OBJECT

public:
    PdfDoc(QString pathToPdf = "");
    ~PdfDoc();
    void loadDocument();
    Poppler::Page * getPage(int pageNumber) const;
    QSize getPageSize(int const pageNumber) const;
    int getNextSlideIndex(int const index) const;
    int getPreviousSlideEnd(int const index) const;
    void setPagePart(int const state);
    Poppler::Document const * getDoc() const;

private:
    Poppler::Document * popplerDoc = nullptr;
    QString pdfPath;
    QList<Poppler::Page*> pdfPages;
    QList<QString> labels;
    int pagePart = 0;

signals:

public slots:
};

#endif // PDFWIDGET_H
