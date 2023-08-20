// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <map>
#include <algorithm>
#include <cstring>
#include <QSize>
#include <QIcon>
#include <QPixmap>
#include <QEvent>
#include <QImageReader>
#include "src/gui/actionbutton.h"
#include "src/preferences.h"
#include "src/master.h"
#include "src/gui/toolselectorwidget.h"

const char *action_to_theme_icon(const Action action) noexcept
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
    //case SelectionToBackground: return "?";
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
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    connect(this, &QToolButton::clicked, this, &ActionButton::onClicked);
    connect(this, &ActionButton::sendAction, master(), &Master::handleAction, Qt::QueuedConnection);
}

ActionButton::ActionButton(const Action action, ToolSelectorWidget *parent) :
    ActionButton(parent)
{
    addAction(action);
    display_action = action;
    setToolTip(tr(action_to_description(action)));
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
    const int px = std::min(width(), height()) - 1;
    setIconSize({px,px});

    // Only weird custom icons require update
    if (display_status >= 0)
        setIcon(QIcon(preferences()->icon_path + "/actions/" + action_to_custom_icons(display_action).value(display_status)));
    else
        update();
}

void ActionButton::addAction(const Action action)
{
    if (action == InvalidAction)
        return;
    actions.insert(action);
    if (icon().isNull())
    {
        const QString &name(action_to_theme_icon(action));
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

const char *action_to_description(const Action action) noexcept
{
    switch (action)
    {
    // Navigation actions
    case Update:
        return QT_TRANSLATE_NOOP("ActionButton", "update view (use if e.g. slide has bad resolution after resizing the window)");
    case NextPage:
        return QT_TRANSLATE_NOOP("ActionButton", "go to next page");
    case PreviousPage:
        return QT_TRANSLATE_NOOP("ActionButton", "go to previous page");
    case NextSkippingOverlays:
        return QT_TRANSLATE_NOOP("ActionButton", "go to next page which has not the same page label as the current page");
    case PreviousSkippingOverlays:
        return QT_TRANSLATE_NOOP("ActionButton", "go to previous page which has not the same page label as the current page");
    case FirstPage:
        return QT_TRANSLATE_NOOP("ActionButton", "go to first page");
    case LastPage:
        return QT_TRANSLATE_NOOP("ActionButton", "go to last page");
    // Drawing
    case UndoDrawing:
        return QT_TRANSLATE_NOOP("ActionButton", "undo last drawing/erasing step in the presentation on the current slide");
    case UndoDrawingLeft:
        return QT_TRANSLATE_NOOP("ActionButton", "undo last drawing/erasing step on the left half of a PDF document which is split into notes and a presentation");
    case UndoDrawingRight:
        return QT_TRANSLATE_NOOP("ActionButton", "undo last drawing/erasing step on the right half of a PDF document which is split into notes and a presentation");
    case RedoDrawing:
        return QT_TRANSLATE_NOOP("ActionButton", "redo undone drawing/erasing step in the presentation on the current slide");
    case RedoDrawingLeft:
        return QT_TRANSLATE_NOOP("ActionButton", "redo last drawing/erasing step on the left half of a PDF document which is split into notes and a presentation");
    case RedoDrawingRight:
        return QT_TRANSLATE_NOOP("ActionButton", "redo last drawing/erasing step on the right half of a PDF document which is split into notes and a presentation");
    case ClearDrawing:
        return QT_TRANSLATE_NOOP("ActionButton", "clear all drawings on the current presentation slide");
    case ClearDrawingLeft:
        return QT_TRANSLATE_NOOP("ActionButton", "clear all drawings of the current slide on the left half of a PDF document which is split into notes and a presentation");
    case ClearDrawingRight:
        return QT_TRANSLATE_NOOP("ActionButton", "clear all drawings of the current slide on the right half of a PDF document which is split into notes and a presentation");
    case ScrollDown:
        return QT_TRANSLATE_NOOP("ActionButton", "scroll down presentation view: adds extra space below the slide for drawing");
    case ScrollUp:
        return QT_TRANSLATE_NOOP("ActionButton", "scroll up presentation view");
    case ScrollNormal:
        return QT_TRANSLATE_NOOP("ActionButton", "reset view after scrolling to show full slide again");
    case SaveDrawings:
        return QT_TRANSLATE_NOOP("ActionButton", "save drawings, times and notes to file");
    case SaveDrawingsAs:
        return QT_TRANSLATE_NOOP("ActionButton", "ask where to save drawings, times and notes");
    case LoadDrawings:
        return QT_TRANSLATE_NOOP("ActionButton", "load drawings etc. from file");
    case LoadDrawingsNoClear:
        return QT_TRANSLATE_NOOP("ActionButton", "load drawings etc. from file without first clearing existing drawings");
    // Modify drawn items
    case CopyClipboard:
        return QT_TRANSLATE_NOOP("ActionButton", "copy selected items to the clipboard");
    case CutClipboard:
        return QT_TRANSLATE_NOOP("ActionButton", "remove selected items and copy them to the clipboard");
    case PasteClipboard:
        return QT_TRANSLATE_NOOP("ActionButton", "paste from clipboard");
    case SelectionToForeground:
        return QT_TRANSLATE_NOOP("ActionButton", "bring selected items to the foreground");
    case SelectionToBackground:
        return QT_TRANSLATE_NOOP("ActionButton", "bring selected items to the background");
    case RemoveSelectedItems:
        return QT_TRANSLATE_NOOP("ActionButton", "remove selected items");
    case SelectAll:
        return QT_TRANSLATE_NOOP("ActionButton", "select everything on the current slide");
    case ClearSelection:
        return QT_TRANSLATE_NOOP("ActionButton", "clear selection (select nothing)");
    // Timer
    case StartTimer:
        return QT_TRANSLATE_NOOP("ActionButton", "start or resume presentation timer");
    case StopTimer:
        return QT_TRANSLATE_NOOP("ActionButton", "pause presentation timer");
    case StartStopTimer:
        return QT_TRANSLATE_NOOP("ActionButton", "pause or continue presentation timer");
    case ResetTimePassed:
        return QT_TRANSLATE_NOOP("ActionButton", "reset timer: sets passed time to zero");
    // Media
    case PlayMedia:
        return QT_TRANSLATE_NOOP("ActionButton", "play all videos on the current slide");
    case PauseMedia:
        return QT_TRANSLATE_NOOP("ActionButton", "pause all videos on the current slide");
    case PlayPauseMedia:
        return QT_TRANSLATE_NOOP("ActionButton", "If any video is playing: pause all videos. Otherwise: start all videos.");
    case Mute:
        return QT_TRANSLATE_NOOP("ActionButton", "mute all media objects");
    case Unmute:
        return QT_TRANSLATE_NOOP("ActionButton", "unmute all media objects");
    // Other actions
    case ReloadFiles:
        return QT_TRANSLATE_NOOP("ActionButton", "reload PDF file(s)");
    case FullScreen:
        return QT_TRANSLATE_NOOP("ActionButton", "toggle fullscreen mode for currently active window");
    case Quit:
        return QT_TRANSLATE_NOOP("ActionButton", "quit and ask to save drawings if there are unsaved changes. Detection of unsaved changes is not reliable yet.");
    case QuitNoConfirmation:
        return QT_TRANSLATE_NOOP("ActionButton", "quit without asking to save drawings");
    default:
        return "";
    };
}
