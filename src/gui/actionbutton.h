#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QToolButton>
#include "src/enumerates.h"

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
    explicit ActionButton(QWidget *parent = NULL);

    explicit ActionButton(const Action action, QWidget *parent = NULL);

    void addAction(const Action action);

protected:
    void onClicked() const noexcept
    {for (const auto action : actions) emit sendAction(action);}

signals:
    void sendAction(const Action action) const;
};

/// Map Actions to icon names.
static const QMap<Action, QString> action_to_theme_icon {
    {PlayMedia, "media-playback-start"},
    {PauseMedia, "media-playback-stop"},
    {PlayPauseMedia, "media-playback-pause"},
    {NextPage, "go-next"},
    {PreviousPage, "go-previous"},
    {FirstPage, "go-first"},
    {LastPage, "go-last"},
    {PreviousSkippingOverlays, "go-up"},
    {NextSkippingOverlays, "go-down"},
    {Quit, "application-exit"},
    {FullScreen, "view-fullscreen"},
    {Update, "view-refresh"},
    {ClearDrawing, "edit-delete"},
    {SaveDrawings, "document-save"},
    {LoadDrawings, "document-open"},
    {UndoDrawing, "edit-undo"},
    {RedoDrawing, "edit-redo"},
    {Mute, "audio-volume-muted"},
    {Unmute, "audio-volume-high"},
    // TODO: more and better icons
};


#endif // ACTIONBUTTON_H
