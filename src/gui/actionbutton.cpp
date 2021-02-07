#include "actionbutton.h"


ActionButton::ActionButton(QWidget *parent) :
    QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, &ActionButton::onClicked);
}
