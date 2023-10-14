// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QSet>
#include <QToolButton>
#include <QStringList>
#include "src/config.h"
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
    /// Action represented by the currently visible icon.
    Action display_action;
    /// Status index of the currently displayed action.
    /// Some actions have multiple icons, depending on this status index.
    int display_status = -1;

public:
    /// Constructor: connect to parent.
    explicit ActionButton(ToolSelectorWidget *parent = nullptr);

    /// Constructor: connect to parent, add action and set icon.
    explicit ActionButton(const Action action, ToolSelectorWidget *parent = nullptr);

    /// Add new action to actions, set icon if necessary.
    void addAction(const Action action);

protected slots:
    /// Send out action(s).
    void onClicked() const noexcept
    {for (const auto action : actions) emit sendAction(action);}

public slots:
    /// Set status for given action. This changes the icon for some actions.
    void setStatus(const Action action, const int status);

    /// Update the icon.
    void updateIcon();

signals:
    /// Send out an action.
    void sendAction(const Action action) const;
};

/// Map actions to icon names.
const char *action_to_theme_icon(const Action action) noexcept;

/**
 * Map actions to lists of custom icon file names.
 * If the list of file names contains multiple elements, the icon is changed
 * based on the status of the corresponding action.
 */
const QStringList action_to_custom_icons(const Action action) noexcept;


/// Get description for action
const char *action_to_description(const Action action) noexcept;

#endif // ACTIONBUTTON_H
