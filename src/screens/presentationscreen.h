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

#include <QKeyEvent>
#include <QWheelEvent>
#include <QGridLayout>
#include "../pdf/pdfdoc.h"
#include "../slide/presentationslide.h"
#include "../enumerates.h"

class PresentationScreen : public QWidget
{
    Q_OBJECT
    friend class ControlScreen;

public:
    explicit PresentationScreen(PdfDoc* presentationDoc, PagePart const part, QWidget* parent = nullptr);
    ~PresentationScreen() override;
    void renderPage(int pageNumber = 0, bool const setDuration = true);
    int getPageNumber() const {return slide->pageNumber();}
    void updatedFile();
    void setScrollDelta(int const scrollDelta) {this->scrollDelta=scrollDelta;}
    void setForceTouchpad() {forceIsTouchpad=true;}

protected:
    void keyPressEvent(QKeyEvent* event) override {emit sendKeyEvent(event);}
    void resizeEvent(QResizeEvent*) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QGridLayout* layout;
    PdfDoc* presentation;
    PresentationSlide* slide;
    int numberOfPages;
    int pageIndex = 0;
    bool forceIsTouchpad = false;
    int scrollDelta = 200;
    int scrollState = 0;
    bool cacheVideos = true;

signals:
    void sendNewPageNumber(const int pageNumber);
    void sendCloseSignal();
    void sendKeyEvent(QKeyEvent* event);
    void clearPresentationCacheRequest();
    void pageChanged(int const pageNumber);

public slots:
    void receiveTimeoutSignal() {renderPage(slide->pageNumber() + 1, true);}
    void receiveNewPage(int const pageNumber) {renderPage(pageNumber);}
};

#endif // PRESENTATIONSCREEN_H
