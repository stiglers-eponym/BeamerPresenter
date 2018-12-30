/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "controlscreen.h"
#include "ui_controlscreen.h"

ControlScreen::ControlScreen(QString presentationPath, QString notesPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControlScreen)
{
    ui->setupUi(this);
    presentation = new PdfDoc( presentationPath );
    presentation->loadDocument();
    notes = new PdfDoc( notesPath );
    notes->loadDocument();
    numberOfPages = presentation->popplerDoc->numPages();
    ui->text_number_slides->setText( QString::fromStdString( std::to_string(numberOfPages) ) );
    ui->text_current_slide->setNumberOfPages(numberOfPages);
    ui->notes_label->setFocus();

    presentationScreen = new PresentationScreen( presentation );

    QObject::connect(ui->notes_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    QObject::connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    QObject::connect(ui->next_slide_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    QObject::connect(ui->notes_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    QObject::connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    QObject::connect(ui->next_slide_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);

    QObject::connect(presentationScreen, &PresentationScreen::sendPageShiftReturn, this, &ControlScreen::receivePageShiftReturn);
    QObject::connect(presentationScreen, &PresentationScreen::sendKeyEvent, this, &ControlScreen::keyPressEvent);
    QObject::connect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    QObject::connect(presentationScreen, &PresentationScreen::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    QObject::connect(this, &ControlScreen::sendCloseSignal, presentationScreen, &PresentationScreen::receiveCloseSignal);
    QObject::connect(presentationScreen, &PresentationScreen::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);

    ui->label_timer->setTimerWidget( ui->edit_timer );
    QObject::connect(ui->label_timer, &Timer::sendAlert, this, &ControlScreen::receiveTimerAlert);
    QObject::connect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    QObject::connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit, this, &ControlScreen::receiveNewPageNumber);
    QObject::connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    QObject::connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit, this, &ControlScreen::receivePageShiftEdit);
    QObject::connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn, this, &ControlScreen::receivePageShiftReturn);
    QObject::connect(ui->text_current_slide, &PageNumberEdit::sendEscape, this, &ControlScreen::resetFocus);
    QObject::connect(ui->label_timer, &Timer::sendEscape, this, &ControlScreen::resetFocus);

    QObject::connect(this, &ControlScreen::sendTimerString, ui->label_timer, &Timer::receiveTimerString);
}

ControlScreen::~ControlScreen()
{
    delete notes;
    delete presentationScreen;
    delete ui;
}

void ControlScreen::recalcLayout(const int pageNumber)
{
    double screenRatio = double(height()) / width();
    QSize notesSize = notes->getPageSize(pageNumber);
    double notesSizeRation = double(notesSize.height()) / notesSize.width();
    double relativeNotesWidth = notesSizeRation / screenRatio;
    if (relativeNotesWidth > 0.75)
        relativeNotesWidth = 0.75;
    ui->notes_label->setGeometry(0, 0, int(relativeNotesWidth*width()), height());
    ui->current_slide_label->setMaximumWidth( int( (1-relativeNotesWidth)*width() ) );
    ui->next_slide_label->setMaximumWidth( int( (1-relativeNotesWidth)*width() ) );
    ui->gridLayout->setColumnStretch(0, int( relativeNotesWidth*width() ));
    ui->gridLayout->setColumnStretch(1, int( (1-relativeNotesWidth)*width() ));
    updateGeometry();
}

void ControlScreen::resetFocus()
{
    if (currentPageNumber != presentationScreen->getPageNumber())
        renderPage( presentationScreen->getPageNumber() );
    ui->notes_label->setFocus();
}

void ControlScreen::receiveTimerAlert()
{
    QPalette palette = QPalette();
    palette.setColor(QPalette::Background, Qt::red);
    palette.setColor(QPalette::Base, Qt::red);
    ui->edit_timer->setPalette(palette);
}

void ControlScreen::resetTimerAlert()
{
    QPalette palette = QPalette();
    palette.setColor(QPalette::Background, Qt::darkGray);
    palette.setColor(QPalette::Base, Qt::white);
    ui->edit_timer->setPalette(palette);
}

void ControlScreen::renderPage( int const pageNumber )
{
    currentPageNumber = pageNumber;
    recalcLayout(pageNumber);
    if (pageNumber<0) {
        currentPageNumber = 0;
    }
    else if (pageNumber >= numberOfPages) {
        currentPageNumber = numberOfPages - 1;
    }
    ui->current_slide_label->renderPage( presentation->getPage(currentPageNumber) );
    if (currentPageNumber + 1 < presentation->popplerDoc->numPages())
        ui->next_slide_label->renderPage( presentation->getPage(currentPageNumber+1) );
    else
        ui->next_slide_label->renderPage( presentation->getPage(currentPageNumber) );
    if (currentPageNumber < notes->popplerDoc->numPages())
        ui->notes_label->renderPage( notes->getPage(currentPageNumber) );
    ui->text_current_slide->setText( QString::fromStdString( std::to_string(currentPageNumber+1) ) );
}

void ControlScreen::receiveNewPageNumber(int const pageNumber)
{
    renderPage( pageNumber );
}

void ControlScreen::receivePageShiftEdit(int const shift)
{
    renderPage( currentPageNumber + shift );
}

void ControlScreen::receivePageShiftReturn(int const shift)
{
    int pageNumber = presentationScreen->getPageNumber() + shift;
    emit sendNewPageNumber( pageNumber );
    renderPage( pageNumber );
    ui->label_timer->continueTimer();
}

void ControlScreen::receiveCloseSignal()
{
    close();
}

void ControlScreen::keyPressEvent( QKeyEvent * event )
{
    switch ( event->key() ) {
        case Qt::Key_Right:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
            renderPage( ++currentPageNumber );
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            renderPage( --currentPageNumber );
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_Space:
            renderPage( currentPageNumber );
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_End:
            currentPageNumber = numberOfPages - 1;
            renderPage( currentPageNumber );
            emit sendNewPageNumber( currentPageNumber );
        break;
        case Qt::Key_Home:
            currentPageNumber = 0;
            renderPage( currentPageNumber );
            emit sendNewPageNumber( currentPageNumber );
        break;
        case Qt::Key_Q:
            emit sendCloseSignal();
            close();
        break;
        case Qt::Key_G:
            ui->text_current_slide->setText("");
            ui->text_current_slide->setFocus();
        break;
        case Qt::Key_P:
            ui->label_timer->pauseTimer();
        break;
        case Qt::Key_R:
            ui->label_timer->resetTimer();
        break;
    }
    event->accept();
}
