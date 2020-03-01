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

#include "presentationscreen.h"

PresentationScreen::PresentationScreen(PdfDoc* presentationDoc, PagePart const part, QWidget* parent) :
    QWidget(parent),
    layout(new QGridLayout(this)),
    presentation(presentationDoc),
    slide(new PresentationSlide(presentationDoc, part, this))
{
    //setAttribute(Qt::WA_NativeWindow);
    setGeometry(0, 0, 1920, 1080);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(120, 90);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QPalette::Shadow);
    setPalette(palette);
    numberOfPages = presentationDoc->getDoc()->numPages();

    // slide will contain the slide as a pixmap
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(slide, 0, 0);
    slide->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(slide, &PresentationSlide::sendNewPageNumber, this, &PresentationScreen::receiveNewPage);
    connect(slide, &PresentationSlide::timeoutSignal,     this, &PresentationScreen::receiveTimeoutSignal);
    slide->getPathOverlay()->hidePointer();
#ifdef Q_OS_UNIX
    setWindowIcon(QIcon(ICON_PATH "beamerpresenter.svg"));
#endif
    show();
}

PresentationScreen::~PresentationScreen()
{
    slide->disconnect();
    disconnect();
    delete slide->getCacheMap();
    slide->overwriteCacheMap(nullptr);
    delete slide;
    delete layout;
}

void PresentationScreen::renderPage(int pageNumber, bool const setDuration)
{
#ifdef DEBUG_RENDERING
    qDebug() << "Render page" << pageNumber << setDuration;
#endif
    if (pageNumber < 0 || pageNumber >= numberOfPages)
        pageNumber = numberOfPages - 1;
    slide->renderPage(pageNumber, setDuration);
    if (pageNumber != pageIndex) {
        pageIndex = pageNumber;
        emit pageChanged(pageIndex);
    }
}

void PresentationScreen::resizeEvent(QResizeEvent*)
{
#ifdef DEBUG_RENDERING
    qDebug() << "Resize presentation screen" << size();
#endif
    if (slide->getCacheMap() != nullptr)
        slide->getCacheMap()->clearCache();
    slide->renderPage(slide->pageNumber(), false);
    emit presentationResizeEvent();
}

void PresentationScreen::wheelEvent(QWheelEvent* event)
{
    // Handle mouse wheel or touch pad scrolling events.

    // Change the signs in the beginning, this makes the rest less confusing.
    int deltaPix = -event->pixelDelta().y();
    int deltaAngle = -event->angleDelta().y();
    int deltaPages;
    // If a touch pad was used for scrolling:
    if (deltaPix != 0) {
        scrollState += deltaPix;
        deltaPages = scrollState / scrollDelta;
        if (deltaPages<0)
            deltaPages++;
        scrollState -= scrollDelta*deltaPages;
    }
    else if (forceIsTouchpad) {
        scrollState += deltaAngle;
        deltaPages = scrollState / scrollDelta;
        if (deltaPages<0)
            deltaPages++;
        scrollState -= scrollDelta*deltaPages;
    }
    // If a mouse wheel was used for scrolling:
    else {
        if (deltaAngle > 120)
            deltaPages = deltaAngle / 120;
        else if (deltaAngle > 0)
            deltaPages = 1;
        else if (deltaAngle < -120)
            deltaPages = -((-deltaAngle) / 120);
        else if (deltaAngle < 0)
            deltaPages = -1;
        else
            deltaPages = 0;
    }
    if (deltaPages != 0) {
        int currentPage = slide->pageNumber();
        if (deltaPages + currentPage < 0) {
            if (currentPage != 0) {
                renderPage(0, false);
                emit sendNewPageNumber(0);
            }
        }
        else {
            renderPage(currentPage + deltaPages, false);
            emit sendNewPageNumber(currentPage + deltaPages);
        }
    }
    event->accept();
}

void PresentationScreen::updatedFile()
{
    numberOfPages = presentation->getDoc()->numPages();
    slide->clearAll();
}
