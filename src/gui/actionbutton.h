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
 *
 * @see ToolButton
 * @see ToolSelectorWidget
 */
class ActionButton : public QToolButton
{
    Q_OBJECT
    /// Set of one or more actions connected to this button.
    QSet<Action> actions;
    Action display_action;
    int display_status = -1;

    void updateIcon();

public:
    /// Constructor: connect to parent.
    explicit ActionButton(ToolSelectorWidget *parent = NULL);

    /// Constructor: connect to parent, add action and set icon.
    explicit ActionButton(const Action action, ToolSelectorWidget *parent = NULL);

    /// Add new action to actions, set icon if necessary.
    void addAction(const Action action);

protected:
    bool event(QEvent *event) override;

protected slots:
    /// Send out action(s).
    void onClicked() const noexcept;

public slots:
    /// Set status for given action. This changes the icon for some actions.
    void setStatus(const Action action, const int status);

signals:
    /// Send out an action.
    void sendAction(const Action action) const;
};

/// Map actions to icon names.
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

/**
 * Map actions to lists of custom icon file names.
 * If the list of file names contains multiple elements, the icon is changed
 * based on the status of the corresponding action.
 */
static const QMap<Action, QStringList> action_to_custom_icons {
    {StartStopTimer, {"timer-paused.svg", "timer-running.svg"}},
    {StopTimer, {"timer-stop.svg"}},
    {StartTimer, {"timer-start.svg"}},
    {ResetTimePassed, {"timer-reset.svg"}},
    {ReloadFiles, {"reload.svg"}},
    {ScrollUp, {"scroll-up.svg"}},
    {ScrollDown, {"scroll-down.svg"}},
    {ScrollNormal, {"scroll-reset.svg"}},
};


#endif // ACTIONBUTTON_H
