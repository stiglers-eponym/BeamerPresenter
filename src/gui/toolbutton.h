// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>
#include "src/config.h"

class Tool;
class QString;
class QImage;
class QSize;
class QColor;

/**
 * @brief Tool button for drawing and pointing tools.
 *
 * This button does not do anything when it is pushed. Subclasses
 * implement different functions for pushing the button.
 *
 * @see ActionButton
 * @see ToolSelectorButton
 * @see ToolWidgetButton
 */
class ToolButton : public QToolButton
{
    Q_OBJECT

protected:
    /// Tool which remains owned by this class.
    /// Only copies of this tool are send out using sendTool.
    Tool *tool = nullptr;

public:
    /// Constructor: takes ownership of tool.
    explicit ToolButton(Tool *tool, QWidget *parent = nullptr) noexcept;

    /// Destruktor: deletes tool.
    virtual ~ToolButton();

public slots:
    /// Replace tool with newtool. Old tool gets deleted. This takes ownership of newtool.
    void setTool(Tool *newtool);

    /// Update the tool icon.
    void updateIcon();

signals:
    /// Send a copy of tool. Ownership of toolcopy is handed to receiver.
    void sendTool(Tool *toolcopy) const;
};

/// Take an svg file, replace #ff0000 by given color, render it to given size and return the QImage.
const QImage fancyIcon(const QString &filename, const QSize &size, const QColor &color);

#endif // TOOLBUTTON_H
