#include "slidelabelwidget.h"

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
}

void SlideLabelWidget::resizeEvent(QResizeEvent *event) noexcept
{
    const int basesize = std::min(event->size().height()*2/3, event->size().width()/10);
    total->setFont({"", basesize, 2});
    edit->setFont({"", basesize+1, 2});
}

void SlideLabelWidget::updateText(const int page) noexcept
{
    total->setText(" / " + preferences().document->pageLabel(preferences().number_of_pages - 1));
    edit->setText(preferences().document->pageLabel(page));
}

void SlideLabelWidget::readText() noexcept
{
    const int page = preferences().document->pageIndex(edit->text());
    if (page >= 0 && page < preferences().number_of_pages)
        emit navigationSignal(page);
    else
        edit->setText("?");
    updateText(page);
}
