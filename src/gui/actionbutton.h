// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ACTIONBUTTON_H
#define ACTIONBUTTON_H

#include <QObject>
#include <QSet>
#include <QToolButton>
#include <QStringList>
#include "src/config.h"
#include "src/enumerates.h"

class ToolSelectorWidget;
class QEvent;

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
const QString action_to_theme_icon(const Action action) noexcept;

/**
 * Map actions to lists of custom icon file names.
 * If the list of file names contains multiple elements, the icon is changed
 * based on the status of the corresponding action.
 */
const QStringList action_to_custom_icons(const Action action) noexcept;


#endif // ACTIONBUTTON_H
