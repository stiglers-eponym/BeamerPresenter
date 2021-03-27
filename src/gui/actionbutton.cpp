#include "src/gui/actionbutton.h"
#include "src/names.h"

ActionButton::ActionButton(QWidget *parent) :
    QPushButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    connect(this, &QPushButton::clicked, this, &ActionButton::onClicked);
}

ActionButton::ActionButton(const Action action, QWidget *parent) :
    ActionButton(parent)
{
    addAction(action);
    setToolTip(action_to_description.value(action));
}
