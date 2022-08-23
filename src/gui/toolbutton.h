// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include "src/config.h"

class Tool;
class QString;
class QEvent;
class QImage;
class QSize;
class QColor;

/**
 * @brief Tool button for drawing and pointing tools.
 *
 * @see ActionButton
 * @see ToolSelectorWidget
 * @see ToolDialog
 */
class ToolButton : public QToolButton
{
    Q_OBJECT

    /// Tool which remains owned by this class.
    /// Only copies of this tool are send out using sendTool.
    Tool *tool = nullptr;

public:
    /// Constructor: takes ownership of tool.
    explicit ToolButton(Tool *tool, QWidget *parent = nullptr) noexcept;

    /// Destruktor: deletes tool.
    virtual ~ToolButton();

protected:
    /// Emit sendTool based on input event with adjusted device.
    virtual bool event(QEvent *event) noexcept override;

public slots:
    /// Replace tool with newtool. Old tool gets deleted.
    void setTool(Tool *newtool);

signals:
    /// Send a copy of tool. Ownership of toolcopy is handed to receiver.
    void sendTool(Tool *toolcopy) const;
};

/// Take an svg file, replace #ff0000 by given color, render it to given size and return the QImage.
const QImage fancyIcon(const QString &filename, const QSize &size, const QColor &color);

#endif // TOOLBUTTON_H
