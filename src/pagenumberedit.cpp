/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "pagenumberedit.h"

PageNumberEdit::PageNumberEdit(QWidget * parent) : QLineEdit(parent)
{
    connect(this, &PageNumberEdit::textChanged, this, &PageNumberEdit::receiveEditSignal);
}

PageNumberEdit::~PageNumberEdit()
{
    disconnect(this, &PageNumberEdit::textChanged, this, &PageNumberEdit::receiveEditSignal);
}

void PageNumberEdit::setNumberOfPages(const int numberOfPages)
{
    this->numberOfPages = numberOfPages;
}

void PageNumberEdit::receiveReturnSignal()
{
    emit sendPageNumberReturn( text().toInt() - 1 );
}

void PageNumberEdit::receiveEditSignal(const QString string)
{
    int pageNumber = string.toInt();
    if ((pageNumber > 0) && (pageNumber <= numberOfPages))
        emit sendPageNumberEdit( pageNumber - 1 );
}

void PageNumberEdit::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
        case Qt::Key_PageDown:
            emit sendPageShiftReturn(1);
            break;
        case Qt::Key_PageUp:
            emit sendPageShiftReturn(-1);
            break;
        case Qt::Key_Down:
            emit sendPageShiftEdit(1);
            break;
        case Qt::Key_Up:
            emit sendPageShiftEdit(-1);
            break;
        case Qt::Key_End:
            setText( QString::fromStdString( std::to_string( numberOfPages )) );
            emit sendPageNumberEdit( numberOfPages - 1 );
            break;
        case Qt::Key_Home:
            setText( "1" );
            emit sendPageNumberEdit( 0 );
            break;
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Control:
            QLineEdit::keyPressEvent(event);
            break;
        case Qt::Key_Escape:
            emit sendEscape();
            break;
        case Qt::Key_Return:
            emit sendPageNumberReturn( text().toInt() - 1 );
            break;
    }
    event->accept();
}
