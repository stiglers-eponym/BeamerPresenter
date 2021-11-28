#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QToolButton>
#include "src/enumerates.h"

class ToolSelectorWidget;

/**
 * @brief Button which sends Action(s) when clicked.
 *
 * When clicked, this emits sendAction for all actions added to this
 * in no specific order.
 */
class ActionButton : public QToolButton
{
    Q_OBJECT
    /// Set of one or more actions connected to this button.
    QSet<Action> actions;

public:
    explicit ActionButton(ToolSelectorWidget *parent = NULL);

    explicit ActionButton(const Action action, ToolSelectorWidget *parent = NULL);

    void addAction(const Action action);

protected slots:
    void onClicked() const noexcept
    {for (const auto action : actions) emit sendAction(action);}

public slots:
    void setStatus(const Action action, const int status);

signals:
    void sendAction(const Action action) const;
};

/// Map Actions to icon names.
static const QMap<Action, QString> action_to_theme_icon {
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
    {ClearDrawing, "edit-delete"},
    {SaveDrawings, "document-save"},
    {SaveDrawingsAs, "document-save-as"},
    {LoadDrawings, "document-open"},
    //{LoadDrawingsNoClear, "document-open"},
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

static const QMap<Action, QStringList> action_to_custom_icons {
    {StartStopTimer, {"actions/timer-paused.svg", "actions/timer-running.svg"}},
    {StopTimer, {"actions/timer-stop.svg"}},
    {StartTimer, {"actions/timer-start.svg"}},
    {ResetTimePassed, {"actions/timer-reset.svg"}},
    {ReloadFiles, {"actions/reload.svg"}},
    {ScrollUp, {"actions/scroll-up.svg"}},
    {ScrollDown, {"actions/scroll-down.svg"}},
    {ScrollNormal, {"actions/scroll-reset.svg"}},
};


#endif // ACTIONBUTTON_H
