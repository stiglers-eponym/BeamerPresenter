/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef CONTROLSCREEN_H
#define CONTROLSCREEN_H

#include <QMainWindow>
#include <QLabel>
#include <QKeyEvent>
#include "src/pdfdoc.h"
#include "src/timer.h"
#include "src/pagenumberedit.h"
#include "src/presentationscreen.h"

namespace Ui {
    class ControlScreen;
}

class ControlScreen : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlScreen(QString presentationPath = "", QString notesPath = "", QWidget *parent = nullptr);
    ~ControlScreen();
    void renderPage(int const pageNumber);

protected:
    void keyPressEvent( QKeyEvent * event );

private:
    void recalcLayout(int const pageNumber);
    Ui::ControlScreen *ui;
    PdfDoc* presentation;
    PdfDoc* notes;
    int numberOfPages;
    int currentPageNumber = 0;
    PresentationScreen* presentationScreen;

signals:
    void sendNewPageNumber(int pageNumber);
    void sendCloseSignal();

public slots:
    void receiveNewPageNumber(int const pageNumber);
    void receivePageShiftEdit(int const shift = 0);
    void receivePageShiftReturn(int const shift = 0);
    void receiveCloseSignal();
    void receiveTimerAlert();
    void resetTimerAlert();
    void resetFocus();
};

#endif // CONTROLSCREEN_H
