#include "tocbutton.h"

TocButton::TocButton(QString const& text, QString const& dest, QWidget * parent) : QPushButton(text, parent)
{
    this->dest = dest;
    setStyleSheet("Text-align:left");
    connect(this, &TocButton::clicked, this, [&](){emit activated(this->dest);});
}
