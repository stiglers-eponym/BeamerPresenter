#include "tocaction.h"

TocAction::TocAction(QString const& text, QString const& dest, QWidget * parent) : QAction(text, parent)
{
    this->dest = dest;
    connect(this, &TocAction::triggered, this, [&](){emit activated(this->dest);});
}
