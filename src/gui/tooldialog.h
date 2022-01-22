#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include <QMap>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include "src/drawing/tool.h"
#include "src/drawing/drawtool.h"

class QCheckBox;
class TextTool;
class PointingTool;

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
    /// Button for color selection (filling color of the tool). The color is stored as the color of the button.
    QPushButton *brush_color_button;
    /// enable/disable filling of paths.
    QCheckBox *fill_checkbox;
    /// Select shape
    QComboBox *shape_box;

public:
    /// Constructor: create layout, use default values from old tool.
    DrawToolDetails(Tool::BasicTool basic_tool, QWidget *parent = NULL, const DrawTool *oldtool = NULL);
    /// Trivial destructor.
    ~DrawToolDetails() {}
    /// @return brush for filling path
    QBrush brush() const;
    /// @return width for stroking path
    qreal width() const
    {return width_box->value();}
    /// @return shape for draw tool
    DrawTool::Shape shape() const
    {return string_to_shape.value(shape_box->currentText(), DrawTool::Freehand);}

public slots:
    /// Choose color using a color dialog
    void setBrushColor();
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
    QDoubleSpinBox *scale_box = NULL;

public:
    /// Constructor: create layout, use default values from old tool.
    PointingToolDetails(Tool::BasicTool basic_tool, QWidget *parent = NULL, const PointingTool *oldtool = NULL);
    /// Trivial destructor.
    ~PointingToolDetails() {}
    /// @return scale property of pointing tool
    float scale() const {return scale_box ? scale_box->value() : -1.;}
    /// @return radius (size) of the pointing tool
    qreal radius() const {return radius_box->value();}
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
    TextToolDetails(QWidget *parent = NULL, const TextTool *oldtool = NULL);
    /// Trivial destructor.
    ~TextToolDetails() {}
    /// @return font of the text input tool
    QFont font() const {return font_button->font();}

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
    QWidget *tool_specific = NULL;
    /// select basic tool
    QComboBox *tool_box;
    /// select color (opens QColorDialog)
    QPushButton *color_button = NULL;
    /// list of checkboxes for input devices
    QMap<int, QCheckBox*> device_buttons;

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicTool(const Tool::BasicTool tool);

public:
    /// Constructor: initialize general tool selector.
    ToolDialog(QWidget *parent = NULL);

    /// Adjust current settings to values of tool.
    void setDefault(const Tool *tool);

    /// Create and return a new tool based on current settings.
    Tool *createTool() const;

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicToolStr(const QString &text)
    {adaptToBasicTool(string_to_tool.value(text));}

    /// Open a new dialog to select a tool.
    /// Default settings are taken from oldtool (if it exists).
    /// Return NULL if basic_tool is invalid.
    static Tool *selectTool(const Tool *oldtool = NULL);

public slots:
    /// Set color button color from a color dialog.
    void setColor();
};

#endif // TOOLDIALOG_H
