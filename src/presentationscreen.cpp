/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "presentationscreen.h"

PresentationScreen::PresentationScreen(PdfDoc* presentationDoc, QWidget *parent) : QWidget(parent)
{
    presentation = presentationDoc;
    setGeometry(0, 0, 1920, 1080);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setMinimumSize(120, 90);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QPalette::Shadow);
    setPalette(palette);

    // label will contain the slide as a pixmap
    label = new PageLabel(this);
    label->setAlignment(Qt::AlignCenter);
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( label, 0, 0 );
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(label, &PageLabel::sendNewPageNumber,       this, &PresentationScreen::receiveNewPageNumber);
    connect(label, &PageLabel::sendNewPageNumber,       this, &PresentationScreen::sendNewPageNumber);
    connect(label, &PageLabel::timeoutSignal,           this, &PresentationScreen::receiveTimeoutSignal);
    connect(this, &PresentationScreen::togglePointerVisibilitySignal, label, &PageLabel::togglePointerVisibility);
    label->togglePointerVisibility();
    show();
}

PresentationScreen::~PresentationScreen()
{
    disconnect(label, &PageLabel::sendNewPageNumber,       this, &PresentationScreen::receiveNewPageNumber);
    disconnect(label, &PageLabel::sendNewPageNumber,       this, &PresentationScreen::sendNewPageNumber);
    disconnect(label, &PageLabel::timeoutSignal,           this, &PresentationScreen::receiveTimeoutSignal);
    disconnect(this, &PresentationScreen::togglePointerVisibilitySignal, label, &PageLabel::togglePointerVisibility);
    delete label;
    delete presentation;
    delete layout;
}

PageLabel * PresentationScreen::getLabel()
{
    return label;
}

int PresentationScreen::getPageNumber() const
{
    return label->pageNumber();
}

void PresentationScreen::renderPage( const int pageNumber )
{
    if ( pageNumber < 0 || pageNumber >= presentation->popplerDoc->numPages() )
        label->renderPage( presentation->getPage( presentation->popplerDoc->numPages() - 1 ) );
    else
        label->renderPage( presentation->getPage(pageNumber) );
}

void PresentationScreen::updateCache()
{
    int pageNumber = label->pageNumber() + 1;
    if (pageNumber < presentation->popplerDoc->numPages())
        label->updateCache( presentation->getPage(pageNumber) );
}

void PresentationScreen::receiveTimeoutSignal()
{
    renderPage( label->pageNumber() + 1 );
    if ( label->getDuration() < 0 || label->getDuration() > 0.5 )
        emit sendPageShift();
}

void PresentationScreen::receiveNewPageNumber( const int pageNumber )
{
    renderPage(pageNumber);
}

void PresentationScreen::receiveCloseSignal()
{
    close();
}

void PresentationScreen::keyPressEvent( QKeyEvent * event )
{
    // TODO: Find a nicer way to do this
    switch ( event->key() ) {
        case Qt::Key_Right:
        case Qt::Key_PageDown:
            renderPage( label->pageNumber() + 1 );
            if ( label->getDuration() < 0 || label->getDuration() > 0.5 )
                emit sendPageShift();
            updateCache();
            if ( label->getDuration() < 0 || label->getDuration() > 0.5 )
                emit sendUpdateCache();
            break;
        case Qt::Key_Left:
        case Qt::Key_PageUp:
            {
                int page = label->pageNumber() - 1;
                if (page >= 0) {
                    renderPage( label->pageNumber() - 1 );
                    if ( label->getDuration() < 0 || label->getDuration() > 0.5 )
                        emit sendPageShift();
                    updateCache();
                    if ( label->getDuration() < 0 || label->getDuration() > 0.5 )
                        emit sendUpdateCache();
                }
            }
            break;
        //case Qt::Key_E: // For those who prefere vim shortcuts
        case Qt::Key_Down:
            {
                int pageNumber = presentation->getNextSlideIndex(label->pageNumber());
                renderPage( pageNumber );
                emit sendNewPageNumber( pageNumber );
                updateCache();
                emit sendUpdateCache();
            }
            break;
        //case Qt::Key_B: // For those who prefere vim shortcuts
        case Qt::Key_Up:
            {
                int pageNumber = presentation->getPreviousSlideEnd(label->pageNumber());
                renderPage( pageNumber );
                emit sendNewPageNumber( pageNumber );
                updateCache();
                emit sendUpdateCache();
            }
            break;
        case Qt::Key_Space:
            renderPage( label->pageNumber() );
            emit sendPageShift();
            updateCache();
            emit sendUpdateCache();
            break;
        case Qt::Key_O:
            emit togglePointerVisibilitySignal();
            break;
        case Qt::Key_F:
        case Qt::Key_F11:
            if (this->windowState() == Qt::WindowFullScreen)
                showNormal();
            else
                showFullScreen();
            break;
        default:
            emit sendKeyEvent(event);
            break;
    }
    event->accept();
}

void PresentationScreen::resizeEvent(QResizeEvent *event)
{
    label->clearCache();
    label->renderPage( label->getPage() );
}
