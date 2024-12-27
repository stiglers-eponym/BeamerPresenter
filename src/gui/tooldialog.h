// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QMap>
#include <QPushButton>
#include <QString>
#include <memory>

#include "src/config.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/tool.h"

class TextTool;
class PointingTool;
class QCheckBox;

/**
 * @brief DrawToolDetails: details for draw tools in a ToolDialog.
 */
class DrawToolDetails : public QWidget
{
  Q_OBJECT

  /// Default stroke widths for the tools
  static const QMap<Tool::BasicTool, qreal> default_widths;
  /// Double input box for stroke width (in pt)
  QDoubleSpinBox *width_box;
  /// Button for color selection (filling color of the tool). The color is
  /// stored as the color of the button.
  QPushButton *brush_color_button;
  /// enable/disable filling of paths.
  QCheckBox *fill_checkbox;
  /// Select shape
  QComboBox *shape_box;
  /// Select pen style
  QComboBox *pen_style_box;
  /// Select brush style
  QComboBox *brush_style_box;

 public:
  /// Constructor: create layout, use default values from old tool.
  DrawToolDetails(Tool::BasicTool basic_tool, QWidget *parent = nullptr,
                  std::shared_ptr<const DrawTool> oldtool = nullptr);
  /// Trivial destructor.
  ~DrawToolDetails() {}

  /// @return brush for filling path
  QBrush brush() const;

  /// @return width for stroking path
  qreal width() const { return width_box->value(); }

  /// @return pen style for stroking path
  Qt::PenStyle penStyle() const
  {
    return pen_style_box->currentData().value<Qt::PenStyle>();
  }

  /// @return shape for draw tool
  DrawTool::Shape shape() const
  {
    return shape_box->currentData().value<DrawTool::Shape>();
  }

 public slots:
  /// Choose color using a color dialog
  void setBrushColor();
  /// Shape changed by shape_box. Disable/enable brush.
  void changeShape(int index);
  /// Set brush style, apply it to brush color button.
  void setBrushStyle(int index);
};

/**
 * @brief PointingToolDetails: details for pointing tools in a ToolDialog.
 */
class PointingToolDetails : public QWidget
{
  Q_OBJECT

  /// Default sizes for the tools
  static const QMap<Tool::BasicTool, qreal> default_sizes;
  /// Double input box for tool radius (in pt)
  QDoubleSpinBox *radius_box;
  /// Double input box for scale property of pointing tool.
  /// This property is only used for magnifier and eraser.
  QDoubleSpinBox *scale_box = nullptr;

 public:
  /// Constructor: create layout, use default values from old tool.
  PointingToolDetails(Tool::BasicTool basic_tool, QWidget *parent = nullptr,
                      std::shared_ptr<const PointingTool> oldtool = nullptr);

  /// Trivial destructor.
  ~PointingToolDetails() {}

  /// @return scale property of pointing tool
  float scale() const { return scale_box ? scale_box->value() : -1.; }

  /// @return radius (size) of the pointing tool
  qreal radius() const { return radius_box->value(); }
};

/**
 * @brief TextToolDetails: details for text tool in a ToolDialog.
 */
class TextToolDetails : public QWidget
{
  Q_OBJECT

  /// Button to select a font using a QFontDialog
  QPushButton *font_button;

 public:
  /// Constructor: create layout, use default values from old tool.
  TextToolDetails(QWidget *parent = nullptr,
                  std::shared_ptr<const TextTool> oldtool = nullptr);
  /// Trivial destructor.
  ~TextToolDetails() {}
  /// @return font of the text input tool
  QFont font() const { return font_button->font(); }

 public slots:
  /// Select font using QFontDialog
  void selectFont();
};

/**
 * @brief ToolDialog: select tool using GUI
 */
class ToolDialog : public QDialog
{
  Q_OBJECT

  /// Widget containing all tool-specific details
  QWidget *tool_specific = nullptr;
  /// select basic tool
  QComboBox *tool_box;
  /// select color (opens QColorDialog)
  QPushButton *color_button = nullptr;
  /// list of checkboxes for input devices
  QMap<Tool::InputDevices, QCheckBox *> device_buttons;

  /// Adjust selection possibilities according to basic tool.
  void adaptToBasicTool(const Tool::BasicTool tool);

 public:
  /// Constructor: initialize general tool selector.
  ToolDialog(QWidget *parent = nullptr);

  /// Adjust current settings to values of tool.
  void setDefault(std::shared_ptr<const Tool> tool);

  /// Create and return a new tool based on current settings.
  std::shared_ptr<Tool> createTool() const;

  /// Adjust selection possibilities according to basic tool.
  void adaptToBasicToolIdx(const int index)
  {
    adaptToBasicTool(tool_box->itemData(index).value<Tool::BasicTool>());
  }

  /// Open a new dialog to select a tool.
  /// Default settings are taken from oldtool (if it exists).
  /// Return nullptr if basic_tool is invalid.
  static std::shared_ptr<Tool> selectTool(
      std::shared_ptr<const Tool> oldtool = nullptr);

 public slots:
  /// Set color button color from a color dialog.
  void setColor();
};

#endif  // TOOLDIALOG_H
