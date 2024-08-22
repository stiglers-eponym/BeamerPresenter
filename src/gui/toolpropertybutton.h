// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLPROPERTYBUTTON_H
#define TOOLPROPERTYBUTTON_H

#include <QColor>
#include <QComboBox>
#include <QPainter>
#include <memory>
#include <variant>

#include "src/config.h"
#include "src/preferences.h"

class Tool;
class QFont;
class QEvent;

/// Variant bundling information about tool property changes
typedef std::variant<qreal, Qt::PenStyle, Qt::BrushStyle,
                     QPainter::CompositionMode, QColor, QFont>
    tool_variant;

/**
 * @brief Drop down menu for changing a property of a tool.
 *
 * @see ToolSelectorWidget
 */
class ToolPropertyButton : public QComboBox
{
  Q_OBJECT

  /// Width of a drop down arrow in QComboBox.
  static constexpr int down_arrow_width = 12;

 protected:
  /// Device changed by this button.
  /// This is the last device used to press this button.
  int device;

  /// Set device to the device producing this action, then continue with
  /// QComboBox::action.
  bool event(QEvent *event) override;

  /// Set property for given tool.
  virtual void setToolProperty(std::shared_ptr<Tool> tool) const = 0;

  /// Update currently selected tool property based on device.
  virtual void updateTool() { toolChanged(preferences()->currentTool(device)); }

 public slots:
  /// Update currently selected tool property based on tool.
  virtual void toolChanged(std::shared_ptr<const Tool> tool) = 0;

  /// Update the icon.
  virtual void updateIcon();

 protected slots:
  /// Choose tool and call setToolProperty.
  void changed(const int index) const;

 public:
  /// Constructor: adjust some widget properties.
  ToolPropertyButton(QWidget *parent = nullptr);

  /// Trivial destructor.
  ~ToolPropertyButton() {}

 signals:
  /// Notify master/scene that tool has changed.
  void sendUpdatedTool(std::shared_ptr<const Tool> tool) const;

  /// Notify master/scene that tool properties have been updated.
  void sendToolProperties(const tool_variant &properties) const;
};

#endif  // TOOLPROPERTYBUTTON_H
