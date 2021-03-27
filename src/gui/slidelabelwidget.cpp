#include <iostream>
#include <iomanip>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QResizeEvent>
#include "src/preferences.h"
#include "src/gui/slidelabelwidget.h"

SlideLabelWidget::SlideLabelWidget(QWidget *parent) :
    QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(1);
    edit = new QLineEdit("0", this);
    edit->setAlignment(Qt::AlignCenter);
    layout->addWidget(edit);
    total = new QLabel(" / 0", this);
    total->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(total);
    connect(edit, &QLineEdit::returnPressed, this, &SlideLabelWidget::readText);
    total->setToolTip("last page label");
    edit->setToolTip("current page label");
}

void SlideLabelWidget::resizeEvent(QResizeEvent *event) noexcept
{
    const int basesize = std::min(event->size().height()*2/3, event->size().width()/10);
    total->setFont({"", basesize, 2});
    edit->setFont({"", basesize+1, 2});
}

void SlideLabelWidget::updateText(const int page) noexcept
{
    const QString new_label = preferences()->document->pageLabel(page);
    if ((preferences()->global_flags & Preferences::LogSlideChanges) && new_label != edit->text())
    {
        const quint32 msecs_passed = preferences()->msecs_passed == UINT_LEAST32_MAX ? preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time) : preferences()->msecs_passed;
        const QString string = QTime::fromMSecsSinceStartOfDay(msecs_passed + 500).toString(msecs_passed < 3600000 ? "m:ss" : "h:mm:ss");
        std::cout << "Changed page"
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
