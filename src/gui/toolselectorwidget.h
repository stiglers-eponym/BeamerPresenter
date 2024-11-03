// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLSELECTORWIDGET_H
#define TOOLSELECTORWIDGET_H

#include <QPainter>
#include <QWidget>
#include <memory>

#include "src/config.h"
#include "src/enumerates.h"
#include "src/gui/toolpropertybutton.h"

class Tool;
class QSize;
class QResizeEvent;
class QColor;
class QJsonArray;

/**
 * @brief Widget showing grid of buttons
 *
 * Emits sendTool and sendAction when buttons are pressed.
 *
 * @see ActionButton
 * @see ToolSelectorButton
 */
class ToolSelectorWidget : public QWidget
{
  Q_OBJECT

  /// Initialize a tool property button.
  void initializeToolPropertyButton(const QString &type, const QJsonArray &list,
                                    const int row, const int column);

 public:
  /// Constructor: initialize layout.
  explicit ToolSelectorWidget(QWidget *parent = nullptr);

  /// Size hint for layout.
  QSize sizeHint() const noexcept override;

  /// Create and add all buttons from a JSON array.
  /// This JSON array must be an array of arrays (a matrix) of entries
  /// which define a single button.
  void addButtons(const QJsonArray &full_array);

  /// Optimal height depends on width.
  bool hasHeightForWidth() const noexcept override { return true; }

 protected:
  /// ensure equal row height when resizing.
  void resizeEvent(QResizeEvent *event) override { emit updateIcons(); }

 signals:
  /// Notify master/scene that tool properties have been updated.
  void sendToolProperties(const tool_variant &properties);

  /// Send action status to action buttons.
  void sendStatus(const Action action, const int status);

  /// Send a new tool (copy of the tool of a button) to master.
  /// Ownership of tool is transfered to receiver (master).
  void sendTool(std::shared_ptr<Tool> tool);

  /// Notify master that a tool has been updated.
  /// Ownership of tool does not change.
  void updatedTool(std::shared_ptr<Tool> tool);

  /// Child buttons should update icons, called after resizing.
  void updateIcons();
};

#endif  // TOOLSELECTORWIDGET_H
