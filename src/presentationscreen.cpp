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

PresentationScreen::PresentationScreen(PdfDoc* presentationDoc, QWidget* parent) : QOpenGLWidget(parent)
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

    // presWidget will contain the slide as a pixmap
    presWidget = new PresentationWidget(this);
    presWidget->setShowMultimedia(true);
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(presWidget, 0, 0);
    presWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(presWidget, &PageWidget::sendNewPageNumber, this, &PresentationScreen::receiveNewPageNumber);
    connect(presWidget, &PageWidget::sendNewPageNumber, this, &PresentationScreen::sendNewPageNumber);
    connect(presWidget, &PageWidget::timeoutSignal,     this, &PresentationScreen::receiveTimeoutSignal);
    connect(this, &PresentationScreen::togglePointerVisibilitySignal, presWidget, &PageWidget::togglePointerVisibility);
    presWidget->togglePointerVisibility();
    show();
}

PresentationScreen::~PresentationScreen()
{
    presWidget->disconnect();
    delete videoCacheTimer;
    //delete presentation;
    disconnect();
    delete presWidget;
    delete layout;
}

void PresentationScreen::renderPage(int const pageNumber, bool const setDuration)
{
    if (pageNumber < 0 || pageNumber >= numberOfPages) {
        presWidget->renderPage(presentation->getPage(numberOfPages - 1), setDuration);
        pageIndex = numberOfPages - 1;
    }
    else {
        presWidget->renderPage(presentation->getPage(pageNumber), setDuration);
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
        presWidget->updateCacheVideos(presentation->getPage(pageIndex+1));
}

void PresentationScreen::receiveTimeoutSignal()
{
    renderPage(presWidget->pageNumber() + 1, true);
    if (presWidget->getDuration() < 0 || presWidget->getDuration() > 0.5)
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
            renderPage(presWidget->pageNumber() + 1, true);
            if ( presWidget->getDuration() < 0 || presWidget->getDuration() > 0.5 )
                emit sendPageShift();
            if ( presWidget->getDuration() < 0 || presWidget->getDuration() > 0.5 )
                emit sendUpdateCache();
            break;
        case KeyAction::Previous:
            {
                int page = presWidget->pageNumber() - 1;
                if (page >= 0) {
                    renderPage(page, false);
                    if ( presWidget->getDuration() < 0 || presWidget->getDuration() > 0.5 )
                        emit sendPageShift();
                    if ( presWidget->getDuration() < 0 || presWidget->getDuration() > 0.5 )
                        emit sendUpdateCache();
                }
            }
            break;
        case KeyAction::NextCurrentScreen:
            renderPage(presWidget->pageNumber() + 1, true);
            break;
        case KeyAction::PreviousCurrentScreen:
            {
                int page = presWidget->pageNumber() - 1;
                if (page >= 0)
                    renderPage(page, false);
            }
            break;
        case KeyAction::NextSkippingOverlays:
            {
                int pageNumber = presentation->getNextSlideIndex(presWidget->pageNumber());
                renderPage(pageNumber, false);
                emit sendNewPageNumber(pageNumber);
                emit sendUpdateCache();
            }
            break;
        case KeyAction::PreviousSkippingOverlays:
            {
                int pageNumber = presentation->getPreviousSlideEnd(presWidget->pageNumber());
                renderPage(pageNumber, false);
                emit sendNewPageNumber(pageNumber);
                emit sendUpdateCache();
            }
            break;
        case KeyAction::GoToPage:
            emit focusPageNumberEdit();
            break;
        case KeyAction::Update:
            renderPage(presWidget->pageNumber(), false);
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

void PresentationScreen::resizeEvent(QResizeEvent* event)
{
    presWidget->clearCache();
    presWidget->renderPage(presWidget->getPage(), false);
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
        int currentPage = presWidget->pageNumber();
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
    presWidget->clearAll();
}
