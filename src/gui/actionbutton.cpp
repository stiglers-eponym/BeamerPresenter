#include "src/gui/actionbutton.h"
#include "src/names.h"
#include "src/preferences.h"

ActionButton::ActionButton(QWidget *parent) :
    QToolButton(parent)
{
    setMinimumSize(8, 8);
    setFocusPolicy(Qt::NoFocus);
    setIconSize({64,64});
    connect(this, &QToolButton::clicked, this, &ActionButton::onClicked);
}

ActionButton::ActionButton(const Action action, QWidget *parent) :
    ActionButton(parent)
{
    addAction(action);
    setToolTip(action_to_description.value(action));
}

void ActionButton::addAction(const Action action)
{
    if (action == InvalidAction)
        return;
    if (icon().isNull() && action_to_theme_icon.contains(action))
        setIcon(QIcon::fromTheme(action_to_theme_icon[action]));
    actions.insert(action);
}
