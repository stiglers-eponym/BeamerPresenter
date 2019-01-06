/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef PRESENTATIONSCREEN_H
#define PRESENTATIONSCREEN_H

#include <iostream>
#include <QKeyEvent>
#include <QGridLayout>
#include "src/pdfdoc.h"
#include "src/pagelabel.h"

class PresentationScreen : public QWidget
{
    Q_OBJECT

public:
    explicit PresentationScreen(PdfDoc* presentationDoc, QWidget *parent = nullptr);
    ~PresentationScreen();
    void renderPage(const int pageNumber = 0);
    int getPageNumber() const;
    void updateCache();
    int* currentPageNumber;
    PageLabel * getLabel();

protected:
    void keyPressEvent( QKeyEvent * event );

private:
    QGridLayout* layout;
    PdfDoc* presentation;
    PageLabel* label;

signals:
    void sendNewPageNumber(const int pageNumber);
    void sendCloseSignal();
    void sendPageShift(const int shift = 0);
    void sendKeyEvent(QKeyEvent * event);
    void togglePointerVisibilitySignal();
    void sendUpdateCache();

public slots:
    void receiveNewPageNumber(const int pageNumber);
    void receiveCloseSignal();
    void receiveTimeoutSignal();
};

#endif // PRESENTATIONSCREEN_H
