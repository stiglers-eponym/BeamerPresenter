// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLSELECTORWIDGET_H
#define TOOLSELECTORWIDGET_H

#include <QWidget>
#include "src/enumerates.h"

class Tool;

/**
 * @brief Widget showing grid of buttons
 *
 * Emits sendTool and sendAction when buttons are pressed.
 *
 * @see ActionButton
 * @see ToolButton
 */
class ToolSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor: initialize layout.
    explicit ToolSelectorWidget(QWidget *parent = NULL);

    /// Size hint for layout.
    QSize sizeHint() const noexcept override;

    /// Create and add all buttons from a JSON array.
    /// This JSON array must be an array of arrays (a matrix) of entries
    /// which define a single button.
    void addButtons(const QJsonArray &full_array);

    /// Optimal height depends on width.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Override event: set equal row height when resizing.
    bool event(QEvent *event) override;

signals:
    /// Send out action to master.
    void sendAction(const Action action);

    /// Send out new color to master.
    void sendColor(const QColor &action);

    /// Send out new color to master.
    void sendWidth(const qreal width);

    /// Send action status to action buttons.
    void sendStatus(const Action action, const int status);

    /// Send a new tool (copy of the tool of a button).
    /// Ownership of tool is transfered to receiver.
    void sendTool(Tool *tool) const;
};

#endif // TOOLSELECTORWIDGET_H
