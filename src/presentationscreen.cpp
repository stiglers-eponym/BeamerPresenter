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
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QPalette palette = QPalette();
    palette.setColor(QPalette::Window, QPalette::Shadow);
    setPalette(palette);
    label = new PageLabel(this);
    label->setAlignment(Qt::AlignCenter);
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget( label, 0, 0 );
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QObject::connect(label, &PageLabel::sendNewPageNumber, this, &PresentationScreen::receiveNewPageNumber);
    QObject::connect(label, &PageLabel::sendNewPageNumber, this, &PresentationScreen::sendNewPageNumber);
    show();
}

PresentationScreen::~PresentationScreen()
{
    delete label;
    delete presentation;
    delete layout;
}

int PresentationScreen::getPageNumber() const
{
    return label->pageNumber();
}

void PresentationScreen::renderPage( const int pageNumber )
{
    if ( pageNumber < 0 )
        label->renderPage( presentation->getPage(0) );
    else if ( pageNumber >= presentation->popplerDoc->numPages() )
        label->renderPage( presentation->getPage( presentation->popplerDoc->numPages() - 1 ) );
    else
        label->renderPage( presentation->getPage(pageNumber) );
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
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            renderPage( label->pageNumber() + 1 );
            emit sendPageShiftReturn();
        break;
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            renderPage( label->pageNumber() - 1 );
            emit sendPageShiftReturn();
        break;
        case Qt::Key_Space:
            renderPage( label->pageNumber() );
            emit sendPageShiftReturn();
        break;
        default:
            emit sendKeyEvent(event);
        break;
    }
    event->accept();
}
