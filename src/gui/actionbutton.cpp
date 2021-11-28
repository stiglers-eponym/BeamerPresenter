#include "src/gui/actionbutton.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/toolselectorwidget.h"
#include <QImageReader>

ActionButton::ActionButton(ToolSelectorWidget *parent) :
    QToolButton(parent)
{
    setMinimumSize(8, 8);
    setFocusPolicy(Qt::NoFocus);
    setIconSize({64,64});
    connect(this, &QToolButton::clicked, this, &ActionButton::onClicked);
    connect(this, &ActionButton::sendAction, parent, &ToolSelectorWidget::sendAction);
}

ActionButton::ActionButton(const Action action, ToolSelectorWidget *parent) :
    ActionButton(parent)
{
    addAction(action);
    setToolTip(action_to_description.value(action));
    if (action_to_custom_icons.contains(action) && action_to_custom_icons[action].length() > 1)
        connect(parent, &ToolSelectorWidget::sendStatus, this, &ActionButton::setStatus);
}

void ActionButton::setStatus(const Action action, const int status)
{
    if (status >= 0 && actions.contains(action) && action_to_custom_icons.contains(action) && status < action_to_custom_icons[action].length())
    {
        QImageReader reader(preferences()->icon_path + action_to_custom_icons[action].value(status));
        reader.setScaledSize(iconSize());
        setIcon(QPixmap::fromImage(reader.read()));
    }
}

void ActionButton::addAction(const Action action)
{
    if (action == InvalidAction)
        return;
    actions.insert(action);
    if (icon().isNull())
    {
        const QString &name = action_to_theme_icon.value(action);
        if (name.isEmpty())
            setStatus(action, 0);
        else
            setIcon(QIcon::fromTheme(name));
    }
}
