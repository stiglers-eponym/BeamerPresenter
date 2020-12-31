#include "slidenumberwidget.h"

SlideNumberWidget::SlideNumberWidget(QWidget *parent) :
    QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(1);
    edit = new QLineEdit("0", this);
    edit->setAlignment(Qt::AlignCenter);
    layout->addWidget(edit);
    total = new QLabel(" / 0", this);
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
    total->setText(" / " + QString::number(preferences().number_of_pages));
    edit->setText(QString::number(page + 1));
}

void SlideNumberWidget::readText() noexcept
{
    bool ok;
    const int page = edit->text().toInt(&ok);
    if (ok && page > 0 && page <= preferences().number_of_pages)
        emit navigationSignal(page - 1);
    else
        edit->setText("?");
}
