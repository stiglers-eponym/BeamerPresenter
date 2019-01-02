/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#ifndef PAGENUMBEREDIT_H
#define PAGENUMBEREDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>

class PageNumberEdit : public QLineEdit
{
    Q_OBJECT

public:
    PageNumberEdit(QWidget * parent = nullptr);
    ~PageNumberEdit();
    void setNumberOfPages(const int numberOfPages);

protected:
    void keyPressEvent(QKeyEvent * event);

private slots:
    void receiveReturnSignal();
    void receiveEditSignal(const QString string);

signals:
    void sendPageNumberReturn(int const pageNumber);
    void sendPageNumberEdit(int const pageNumber);
    void sendPageShiftReturn(int const shift);
    void sendPageShiftEdit(int const shift);
    void sendEscape();

private:
    int numberOfPages = 0;
};

#endif // PAGENUMBEREDIT_H
