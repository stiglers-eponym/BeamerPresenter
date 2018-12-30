/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
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
    Poppler::Page* getPage(int pageNumber) const;
    QSize getPageSize(int const pageNumber) const;
    //QImage renderPage(int pageNumber, QSize size=QSize(-1,-1)) const;
    //void renderToLabel(QLabel* label, int pageNumber);

    Poppler::Document *popplerDoc;

private:
    QString pdfPath;
    QList<Poppler::Page*> pdfPages;

signals:

public slots:
};

#endif // PDFWIDGET_H
