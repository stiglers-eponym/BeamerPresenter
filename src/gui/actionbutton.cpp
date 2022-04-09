#include <QImageReader>
#include "src/gui/actionbutton.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/toolselectorwidget.h"

ActionButton::ActionButton(ToolSelectorWidget *parent) :
    QToolButton(parent)
{
    setMinimumSize(12, 12);
    setIconSize({32,32});
    setContentsMargins(0,0,0,0);
    setFocusPolicy(Qt::NoFocus);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(this, &QToolButton::clicked, this, &ActionButton::onClicked);
    connect(this, &ActionButton::sendAction, parent, &ToolSelectorWidget::sendAction);
}

ActionButton::ActionButton(const Action action, ToolSelectorWidget *parent) :
    ActionButton(parent)
{
    addAction(action);
    display_action = action;
    setToolTip(tr(action_to_description.value(action)));
    if (action_to_custom_icons.contains(action) && action_to_custom_icons[action].length() > 1)
        connect(parent, &ToolSelectorWidget::sendStatus, this, &ActionButton::setStatus);
}

void ActionButton::setStatus(const Action action, const int status)
{
    if (status >= 0 && actions.contains(action) && action_to_custom_icons.contains(action) && status < action_to_custom_icons[action].length())
    {
        display_action = action;
        display_status = status;
        updateIcon();
    }
}

void ActionButton::updateIcon()
{
    QSize newsize = size();
    setIconSize(newsize);

    // Only weird custom icons require update
    if (display_status >= 0)
    {
        if (newsize.height() > newsize.width())
            newsize.rheight() = newsize.width();
        else
            newsize.rwidth() = newsize.height();
        QImageReader reader(preferences()->icon_path + "/actions/" + action_to_custom_icons[display_action].value(display_status));
        reader.setScaledSize(newsize);
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
        {
            QIcon icon = QIcon::fromTheme(name);
            if (icon.isNull())
                // Sometimes name + "-symbolic" is a reasonable fallback icon.
                icon = QIcon::fromTheme(name + "-symbolic");
            if (!icon.isNull())
                setIcon(icon);
        }
    }
}

void ActionButton::onClicked() const noexcept
{
    for (const auto action : actions)
        emit sendAction(action);
}

bool ActionButton::event(QEvent *event)
{
    if (event->type() == QEvent::Resize)
        updateIcon();
    return QToolButton::event(event);
}
