// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QSize>
#include <QStringList>
#include <QImageReader>
#include "src/gui/actionbutton.h"
#include "src/names.h"
#include "src/preferences.h"
#include "src/gui/toolselectorwidget.h"

const QString action_to_theme_icon(const Action action) noexcept
{
    static const std::map<Action, std::string> lookup_table {
        /* Nagivation actions */
        {Update, "view-refresh"},
        {NextPage, "go-next"},
        {PreviousPage, "go-previous"},
        {PreviousSkippingOverlays, "go-up"},
        {NextSkippingOverlays, "go-down"},
        {FirstPage, "go-first"},
        {LastPage, "go-last"},
        /* Drawing */
        {UndoDrawing, "edit-undo"},
        {RedoDrawing, "edit-redo"},
        {ClearDrawing, "edit-clear"},
        {SaveDrawings, "document-save"},
        {SaveDrawingsAs, "document-save-as"},
        {LoadDrawings, "document-open"},
        //{LoadDrawingsNoClear, "document-open"},
        /* Modify drawn items */
        {CopyClipboard, "edit-copy"},
        {CutClipboard, "edit-cut"},
        {PasteClipboard, "edit-paste"},
        //{SelectionToForeground, "?"},
        //{DuplicateSelectedItems, "?"},
        {RemoveSelectedItems, "edit-delete"},
        {SelectAll, "edit-select-all"},
        //{ClearSelection, "?"},
        /* Media */
        {PlayMedia, "media-playback-start"},
        {PauseMedia, "media-playback-stop"},
        {PlayPauseMedia, "media-playback-pause"},
        {Mute, "audio-volume-muted"},
        {Unmute, "audio-volume-high"},
        /* Other actions */
        {Quit, "application-exit"},
        {QuitNoConfirmation, "application-exit"},
        {FullScreen, "view-fullscreen"},
    };
    const auto find = lookup_table.find(action);
    if (find == lookup_table.end())
        return QString();
    return QString::fromStdString(find->second);
}

const QStringList action_to_custom_icons(const Action action) noexcept
{
    static const std::map<Action, QStringList> lookup_table {
        {StartStopTimer, {"timer-paused.svg", "timer-running.svg"}},
        {StopTimer, {"timer-stop.svg"}},
        {StartTimer, {"timer-start.svg"}},
        {ResetTimePassed, {"timer-reset.svg"}},
        {ReloadFiles, {"reload.svg"}},
        {ScrollUp, {"scroll-up.svg"}},
        {ScrollDown, {"scroll-down.svg"}},
        {ScrollNormal, {"scroll-reset.svg"}},
    };
    const auto find = lookup_table.find(action);
    if (find == lookup_table.end())
        return QStringList();
    return find->second;
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
