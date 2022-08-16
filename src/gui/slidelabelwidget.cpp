// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <QString>
#include <QLabel>
#include <QFont>
#include <QTime>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QResizeEvent>
#include "src/preferences.h"
#include "src/rendering/pdfdocument.h"
#include "src/gui/slidelabelwidget.h"

SlideLabelWidget::SlideLabelWidget(QWidget *parent) :
    QWidget(parent)
{
    setMinimumSize(24, 10);
    setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *layout = new QHBoxLayout(this);
    edit = new QLineEdit("0", this);
    edit->setAlignment(Qt::AlignCenter);
    layout->addWidget(edit);
    total = new QLabel(" / 0", this);
    total->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(total);
    connect(edit, &QLineEdit::returnPressed, this, &SlideLabelWidget::readText);
    total->setToolTip(tr("last page label"));
    edit->setToolTip(tr("current page label"));

    // If logging is activated: print header
    if ((preferences()->global_flags & Preferences::LogSlideChanges))
        std::cout << "Slide label changes:   time old new" << std::endl;
}

void SlideLabelWidget::resizeEvent(QResizeEvent *event) noexcept
{
    const int basesize = std::min(event->size().height()*2/3, event->size().width()/10);
    QFont thefont = edit->font();
    thefont.setPointSize(basesize);
    total->setFont(thefont);
    thefont.setPointSize(basesize+1);
    edit->setFont(thefont);
}

void SlideLabelWidget::updateText(const int page) noexcept
{
    const QString new_label = preferences()->document->pageLabel(page);
    if ((preferences()->global_flags & Preferences::LogSlideChanges) && new_label != edit->text())
    {
        const quint32 msecs_passed = preferences()->msecs_passed == UINT_LEAST32_MAX ? preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time) : preferences()->msecs_passed;
        const QString string = QTime::fromMSecsSinceStartOfDay(msecs_passed + 500).toString(msecs_passed < 3600000 ? "m:ss" : "h:mm:ss");
        std::cout << "Changed page label"
            << std::setw(9) << string.toStdString()
            << std::setw(4) << edit->text().toStdString()
            << std::setw(4) << new_label.toStdString() << std::endl;
    }
    total->setText(" / " + preferences()->document->pageLabel(preferences()->number_of_pages-1));
    edit->setText(new_label);
}

void SlideLabelWidget::readText() noexcept
{
    const int page = preferences()->document->pageIndex(edit->text());
    if (page >= 0 && page < preferences()->number_of_pages)
        emit navigationSignal(page);
    else
        edit->setText("?");
    updateText(page);
}
