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

#ifndef PRESENTATIONSCREEN_H
#define PRESENTATIONSCREEN_H

#include <QtDebug>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGridLayout>
#include "pdfdoc.h"
#include "pagelabel.h"

class PresentationScreen : public QWidget
{
    Q_OBJECT

public:
    explicit PresentationScreen(PdfDoc* presentationDoc, QWidget* parent = nullptr);
    ~PresentationScreen();
    void renderPage(int const pageNumber = 0, bool const setDuration = false);
    int getPageNumber() const {return label->pageNumber();}
    PageLabel* getLabel() {return label;}
    void updatedFile();

protected:
    void keyPressEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);

private:
    QGridLayout* layout;
    PdfDoc* presentation;
    PageLabel* label;
    int numberOfPages;

signals:
    void sendNewPageNumber(const int pageNumber);
    void sendCloseSignal();
    void sendPageShift(const int shift = 0);
    void sendKeyEvent(QKeyEvent* event);
    void togglePointerVisibilitySignal();
    void sendUpdateCache();
    void focusPageNumberEdit();
    void clearPresentationCacheRequest();

public slots:
    void receiveNewPageNumber(const int pageNumber);
    void receiveCloseSignal();
    void receiveTimeoutSignal();
};

#endif // PRESENTATIONSCREEN_H
