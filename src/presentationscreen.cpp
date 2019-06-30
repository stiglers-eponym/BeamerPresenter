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

PresentationScreen::PresentationScreen(PdfDoc* presentationDoc, QWidget* parent) : QWidget(parent)
{
    //setAttribute(Qt::WA_NativeWindow);
    presentation = presentationDoc;
    setGeometry(0, 0, 1920, 1080);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(120, 90);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QPalette::Shadow);
    setPalette(palette);
    numberOfPages = presentationDoc->getDoc()->numPages();
    videoCacheTimer->setSingleShot(true);
    connect(videoCacheTimer, &QTimer::timeout, this, &PresentationScreen::updateVideoCache);

    // slide will contain the slide as a pixmap
    slide = new PresentationSlide(this);
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(slide, 0, 0);
    slide->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(slide, &PresentationSlide::sendNewPageNumber, this, &PresentationScreen::receiveNewPageNumber);
    connect(slide, &PresentationSlide::sendNewPageNumber, this, &PresentationScreen::sendNewPageNumber);
    connect(slide, &PresentationSlide::timeoutSignal,     this, &PresentationScreen::receiveTimeoutSignal);
    connect(this, &PresentationScreen::togglePointerVisibilitySignal, slide, &PresentationSlide::togglePointerVisibility);
    slide->togglePointerVisibility();
    show();
}

PresentationScreen::~PresentationScreen()
{
    slide->disconnect();
    delete videoCacheTimer;
    //delete presentation;
    disconnect();
    delete slide;
    delete layout;
}

void PresentationScreen::renderPage(int const pageNumber, bool const setDuration)
{
    if (pageNumber < 0 || pageNumber >= numberOfPages) {
        slide->renderPage(presentation->getPage(numberOfPages - 1), setDuration);
        pageIndex = numberOfPages - 1;
    }
    else {
        slide->renderPage(presentation->getPage(pageNumber), setDuration);
        pageIndex = pageNumber;
    }
    // Update video cache
    if (cacheVideos)
        videoCacheTimer->start();
    emit pageChanged(presentation->getSlideNumber(pageIndex));
}

void PresentationScreen::updateVideoCache()
{
    if (pageIndex+1 < numberOfPages)
        slide->updateCacheVideos(presentation->getPage(pageIndex+1));
}

void PresentationScreen::receiveTimeoutSignal()
{
    renderPage(slide->pageNumber() + 1, true);
    if (slide->getDuration() < 0 || slide->getDuration() > 0.5)
        emit sendPageShift();
}

void PresentationScreen::receiveNewPageNumber(const int pageNumber)
{
    // TODO: fix duration: define clearly when the duration is set and when not.
    renderPage(pageNumber, pageNumber>pageIndex);
}

void PresentationScreen::receiveCloseSignal()
{
    close();
}

void PresentationScreen::keyPressEvent(QKeyEvent* event)
{
    QMap<int, QList<int>>::iterator map_it = keymap->find(event->key() + static_cast<int>(event->modifiers()));
    if (map_it==keymap->end())
        return;
    for (QList<int>::const_iterator action_it=map_it->cbegin(); action_it!=map_it->cend(); action_it++) {
        switch (event->key()) {
        case KeyAction::Next:
            renderPage(slide->pageNumber() + 1, true);
            if ( slide->getDuration() < 0 || slide->getDuration() > 0.5 )
                emit sendPageShift();
            if ( slide->getDuration() < 0 || slide->getDuration() > 0.5 )
                emit sendUpdateCache();
            break;
        case KeyAction::Previous:
            {
                int page = slide->pageNumber() - 1;
                if (page >= 0) {
                    renderPage(page, false);
                    if ( slide->getDuration() < 0 || slide->getDuration() > 0.5 )
                        emit sendPageShift();
                    if ( slide->getDuration() < 0 || slide->getDuration() > 0.5 )
                        emit sendUpdateCache();
                }
            }
            break;
        case KeyAction::NextCurrentScreen:
            renderPage(slide->pageNumber() + 1, true);
            break;
        case KeyAction::PreviousCurrentScreen:
            {
                int page = slide->pageNumber() - 1;
                if (page >= 0)
                    renderPage(page, false);
            }
            break;
        case KeyAction::NextSkippingOverlays:
            {
                int pageNumber = presentation->getNextSlideIndex(slide->pageNumber());
                renderPage(pageNumber, false);
                emit sendNewPageNumber(pageNumber);
                emit sendUpdateCache();
            }
            break;
        case KeyAction::PreviousSkippingOverlays:
            {
                int pageNumber = presentation->getPreviousSlideEnd(slide->pageNumber());
                renderPage(pageNumber, false);
                emit sendNewPageNumber(pageNumber);
                emit sendUpdateCache();
            }
            break;
        case KeyAction::GoToPage:
            emit focusPageNumberEdit();
            break;
        case KeyAction::Update:
            renderPage(slide->pageNumber(), false);
            emit sendPageShift();
            emit sendUpdateCache();
            break;
        case KeyAction::ToggleCursor:
            emit togglePointerVisibilitySignal();
            break;
        case KeyAction::FullScreen:
            if (this->windowState() == Qt::WindowFullScreen)
                showNormal();
            else
                showFullScreen();
            break;
        default:
            emit sendKeyEvent(event);
            break;
        }
    }
    event->accept();
}

void PresentationScreen::resizeEvent(QResizeEvent*)
{
    slide->clearCache();
    slide->renderPage(slide->getPage(), false);
    emit clearPresentationCacheRequest();
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
