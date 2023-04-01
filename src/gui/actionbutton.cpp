// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <map>
#include <cstring>
#include <QSize>
#include <QIcon>
#include <QPixmap>
#include <QEvent>
#include <QImageReader>
#include "src/gui/actionbutton.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/toolselectorwidget.h"

const QString action_to_theme_icon(const Action action) noexcept
{
    switch (action)
    {
        /* Nagivation actions */
    case Update:
        return "view-refresh";
    case NextPage:
        return "go-next";
    case PreviousPage:
        return "go-previous";
    case PreviousSkippingOverlays:
        return "go-up";
    case NextSkippingOverlays:
        return "go-down";
    case FirstPage:
        return "go-first";
    case LastPage:
        return"go-last";
        /* Drawing */
    case UndoDrawing:
        return "edit-undo";
    case RedoDrawing:
        return "edit-redo";
    case ClearDrawing:
        return "edit-clear";
    case SaveDrawings:
        return "document-save";
    case SaveDrawingsAs:
        return "document-save-as";
    case LoadDrawings:
        return "document-open";
    //case LoadDrawingsNoClear: return "document-open";
        /* Modify drawn items */
    case CopyClipboard:
        return "edit-copy";
    case CutClipboard:
        return "edit-cut";
    case PasteClipboard:
        return "edit-paste";
    //case SelectionToForeground: return "?";
    //case DuplicateSelectedItems: return "?";
    case RemoveSelectedItems:
        return "edit-delete";
    case SelectAll:
        return "edit-select-all";
    //case ClearSelection: return "?";
        /* Media */
    case PlayMedia:
        return "media-playback-start";
    case PauseMedia:
        return "media-playback-stop";
    case PlayPauseMedia:
        return "media-playback-pause";
    case Mute:
        return "audio-volume-muted";
    case Unmute:
        return "audio-volume-high";
        /* Other actions */
    case Quit:
        return "application-exit";
    case QuitNoConfirmation:
        return "application-exit";
    case FullScreen:
        return "view-fullscreen";
    default:
        return "";
    };
}

const QStringList action_to_custom_icons(const Action action) noexcept
{
    switch (action)
    {
    case StartStopTimer:
        return {"timer-paused.svg", "timer-running.svg"};
    case StopTimer:
        return {"timer-stop.svg"};
    case StartTimer:
        return {"timer-start.svg"};
    case ResetTimePassed:
        return {"timer-reset.svg"};
    case ReloadFiles:
        return {"reload.svg"};
    case ScrollUp:
        return {"scroll-up.svg"};
    case ScrollDown:
        return {"scroll-down.svg"};
    case ScrollNormal:
        return {"scroll-reset.svg"};
    default:
        return QStringList();
    };
}


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
    setToolTip(action_to_description(action));
    if (action_to_custom_icons(action).length() > 1)
        connect(parent, &ToolSelectorWidget::sendStatus, this, &ActionButton::setStatus);
}

void ActionButton::setStatus(const Action action, const int status)
{
    if (status >= 0 && actions.contains(action) && status < action_to_custom_icons(action).length())
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
        QImageReader reader(preferences()->icon_path + "/actions/" + action_to_custom_icons(display_action).value(display_status));
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
        const QString &name = action_to_theme_icon(action);
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
