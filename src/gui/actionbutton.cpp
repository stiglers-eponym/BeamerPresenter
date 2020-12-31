#include "actionbutton.h"

ActionButton::ActionButton(const Action action, QWidget *parent) :
    QPushButton(parent),
    action(action)
{
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, &ActionButton::onClicked);
}
