// SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef CONTAINERBASECLASS_H
#define CONTAINERBASECLASS_H

class QWidget;
class QString;

/**
 * @brief Unified interface for adding widgets to a container widget.
 */
class ContainerBaseClass {
public:
    /// Append a new widget to the layout.
    /// title will be ignored by most implementations.
    virtual void addWidgetCommon(QWidget *widget, const QString &title) = 0;

    /// Return this as QWidget
    virtual QWidget *thisWidget() noexcept = 0;
};

#endif // CONTAINERBASECLASS_H
