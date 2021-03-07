#include <iostream>
#include <iomanip>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QResizeEvent>
#include "src/preferences.h"
#include "src/gui/slidenumberwidget.h"

SlideNumberWidget::SlideNumberWidget(QWidget *parent) :
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
    connect(edit, &QLineEdit::returnPressed, this, &SlideNumberWidget::readText);
}

void SlideNumberWidget::resizeEvent(QResizeEvent *event) noexcept
{
    const int basesize = std::min(event->size().height()*2/3, event->size().width()/10);
    total->setFont({"", basesize, 2});
    edit->setFont({"", basesize+1, 2});
}

void SlideNumberWidget::updateText(const int page) noexcept
{
    if ((preferences()->global_flags & Preferences::LogSlideChanges) && edit->text().toInt() != page+1)
    {
        const quint32 msecs_passed = preferences()->msecs_passed == UINT_LEAST32_MAX ? preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time) : preferences()->msecs_passed;
        const QString string = QTime::fromMSecsSinceStartOfDay(msecs_passed + 500).toString(msecs_passed < 3600000 ? "m:ss" : "h:mm:ss");
        std::cout << "Changed page"
            << std::setw(9) << string.toStdString()
            << std::setw(3) << edit->text().toStdString()
            << std::setw(3) << page + 1 << std::endl;
    }
    total->setText(" / " + QString::number(preferences()->number_of_pages));
    edit->setText(QString::number(page + 1));
}

void SlideNumberWidget::readText() noexcept
{
    bool ok;
    const int page = edit->text().toInt(&ok);
    if (ok && page > 0 && page <= preferences()->number_of_pages)
        emit navigationSignal(page - 1);
    else
        edit->setText("?");
}
