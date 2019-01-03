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
    setWindowTitle("BeamerPresenter: " + notesPath);
    presentation = new PdfDoc( presentationPath );
    presentation->loadDocument();
    notes = new PdfDoc( notesPath );
    notes->loadDocument();
    numberOfPages = presentation->popplerDoc->numPages();
    ui->text_number_slides->setText( QString::fromStdString( std::to_string(numberOfPages) ) );
    ui->text_current_slide->setNumberOfPages(numberOfPages);
    ui->notes_label->setPresentationStatus(false);
    ui->current_slide_label->setShowMultimedia(false);
    ui->next_slide_label->setShowMultimedia(false);
    ui->notes_label->setFocus();

    presentationScreen = new PresentationScreen( presentation );
    presentationScreen->setWindowTitle("BeamerPresenter: " + presentationPath);

    // Page requests from the labels:
    // These are emitted if links are clicked.
    // These events are send to ControlScreen and PresentationScreen
    connect(ui->notes_label,         &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    connect(ui->notes_label,         &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->current_slide_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);

    // Navigation signals emitted by PresentationScreen:
    connect(presentationScreen, &PresentationScreen::sendPageShift,     this, &ControlScreen::receivePageShiftReturn);
    connect(presentationScreen, &PresentationScreen::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);

    // Other signals emitted by PresentationScreen
    connect(presentationScreen, &PresentationScreen::sendKeyEvent,    this, &ControlScreen::keyPressEvent);
    connect(presentationScreen, &PresentationScreen::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);

    // Signals sent back to PresentationScreen
    connect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(this, &ControlScreen::sendCloseSignal,   presentationScreen, &PresentationScreen::receiveCloseSignal);

    ui->label_timer->setTimerWidget( ui->edit_timer );
    // Signals emitted by the timer
    connect(ui->label_timer, &Timer::sendAlert,   this, &ControlScreen::receiveTimerAlert);
    connect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    connect(ui->label_timer, &Timer::sendEscape,  this, &ControlScreen::resetFocus);
    // Signals sent back to the timer
    connect(this, &ControlScreen::sendTimerString, ui->label_timer, &Timer::receiveTimerString);
    connect(this, &ControlScreen::sendTimeoutInterval, ui->label_timer, &Timer::receiveTimeoutInterval);

    // Signals sent to the page labels
    connect(this, &ControlScreen::sendAutostartDelay, ui->notes_label, &PageLabel::setAutostartDelay);
    connect(this, &ControlScreen::sendAutostartDelay, presentationScreen->getLabel(), &PageLabel::setAutostartDelay);

    // Signals emitted by the page number editor
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn,  presentationScreen, &PresentationScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit,  this, &ControlScreen::receiveNewPageNumber);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit,   this, &ControlScreen::receivePageShiftEdit);
    connect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn, this, &ControlScreen::receivePageShiftReturn);
    connect(ui->text_current_slide, &PageNumberEdit::sendEscape,          this, &ControlScreen::resetFocus);
}

ControlScreen::~ControlScreen()
{
    disconnect(ui->notes_label,         &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    disconnect(ui->current_slide_label, &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    disconnect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);
    disconnect(ui->notes_label,         &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    disconnect(ui->current_slide_label, &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    disconnect(ui->next_slide_label,    &PageLabel::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);

    // Navigation signals emitted by PresentationScreen:
    disconnect(presentationScreen, &PresentationScreen::sendPageShift,     this, &ControlScreen::receivePageShiftReturn);
    disconnect(presentationScreen, &PresentationScreen::sendNewPageNumber, this, &ControlScreen::receiveNewPageNumber);

    // Other signals emitted by PresentationScreen
    disconnect(presentationScreen, &PresentationScreen::sendKeyEvent,    this, &ControlScreen::keyPressEvent);
    disconnect(presentationScreen, &PresentationScreen::sendCloseSignal, this, &ControlScreen::receiveCloseSignal);

    // Signals sent back to PresentationScreen
    disconnect(this, &ControlScreen::sendNewPageNumber, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    disconnect(this, &ControlScreen::sendCloseSignal,   presentationScreen, &PresentationScreen::receiveCloseSignal);

    ui->label_timer->setTimerWidget( ui->edit_timer );
    // Signals emitted by the timer
    disconnect(ui->label_timer, &Timer::sendAlert,   this, &ControlScreen::receiveTimerAlert);
    disconnect(ui->label_timer, &Timer::sendNoAlert, this, &ControlScreen::resetTimerAlert);
    disconnect(ui->label_timer, &Timer::sendEscape,  this, &ControlScreen::resetFocus);
    // Signals sent back to the timer
    disconnect(this, &ControlScreen::sendTimerString, ui->label_timer, &Timer::receiveTimerString);
    disconnect(this, &ControlScreen::sendTimeoutInterval, ui->label_timer, &Timer::receiveTimeoutInterval);

    // Signals sent to the page labels
    disconnect(this, &ControlScreen::sendAutostartDelay, ui->notes_label, &PageLabel::setAutostartDelay);
    disconnect(this, &ControlScreen::sendAutostartDelay, presentationScreen->getLabel(), &PageLabel::setAutostartDelay);

    // Signals emitted by the page number editor
    disconnect(ui->text_current_slide, &PageNumberEdit::sendPageNumberReturn, presentationScreen, &PresentationScreen::receiveNewPageNumber);
    disconnect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn,  presentationScreen, &PresentationScreen::receiveNewPageNumber);
    disconnect(ui->text_current_slide, &PageNumberEdit::sendPageNumberEdit,  this, &ControlScreen::receiveNewPageNumber);
    disconnect(ui->text_current_slide, &PageNumberEdit::sendPageShiftEdit,   this, &ControlScreen::receivePageShiftEdit);
    disconnect(ui->text_current_slide, &PageNumberEdit::sendPageShiftReturn, this, &ControlScreen::receivePageShiftReturn);
    disconnect(ui->text_current_slide, &PageNumberEdit::sendEscape,          this, &ControlScreen::resetFocus);
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
            //std::cout << "key signal: sending new page number " << currentPageNumber << std::endl;
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            renderPage( --currentPageNumber );
            //std::cout << "key signal: sending new page number " << currentPageNumber << std::endl;
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_Space:
            renderPage( currentPageNumber );
            //std::cout << "key signal: sending new page number " << currentPageNumber << std::endl;
            emit sendNewPageNumber( currentPageNumber );
            ui->label_timer->continueTimer();
        break;
        case Qt::Key_End:
            currentPageNumber = numberOfPages - 1;
            renderPage( currentPageNumber );
            //std::cout << "key signal: sending new page number " << currentPageNumber << std::endl;
            emit sendNewPageNumber( currentPageNumber );
        break;
        case Qt::Key_Home:
            currentPageNumber = 0;
            renderPage( currentPageNumber );
            //std::cout << "key signal: sending new page number " << currentPageNumber << std::endl;
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
