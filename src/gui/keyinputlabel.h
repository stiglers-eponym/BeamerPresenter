// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef KEYINPUTLABEL_H
#define KEYINPUTLABEL_H

#include <QLabel>
#include <QString>
#include <QKeySequence>
#include "src/config.h"
#include "src/enumerates.h"

class Tool;
class QKeyEvent;

/**
 * @brief QLabel for keyboard shortcut input.
 *
 * Keyboard shortcuts registered by this widget are immediately
 * written to preferences.
 */
class KeyInputLabel : public QLabel
{
    Q_OBJECT
    /// Key combinations entered here will be connected to this Action if it is valid.
    Action action = InvalidAction;
    /// Key combinations entered here will be connected to this Tool if it is not nullptr.
    Tool *tool = nullptr;
    /// key sequence
    QKeySequence keys;

public:
    /// Create a new KeyInputLabel for an action.
    explicit KeyInputLabel(const QKeySequence init, const Action action, QWidget *parent = nullptr);
    /// Create a new KeyInputLabel for a tool.
    explicit KeyInputLabel(const QKeySequence init, Tool *tool, QWidget *parent = nullptr);
    /// Destructor: remove this tool from preferences.
    ~KeyInputLabel();

protected:
    /// Set shortcut for associated action or tool.
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Change action or tool. Open a tool dialog if a tool should be chosen.
    void changeAction(const QString &text) noexcept;

signals:
    /// Send out (new) name of action or tool.
    void sendName(const QString &string);
};

#endif // KEYINPUTLABEL_H
