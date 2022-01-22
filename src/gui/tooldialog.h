#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include <QMap>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "src/drawing/tool.h"

class QComboBox;
class QCheckBox;
class TextTool;
class DrawTool;
class PointingTool;

/**
 * @brief DrawToolDetails: details for draw tools in a ToolDialog.
 */
class DrawToolDetails : public QWidget
{
    Q_OBJECT

    static const QMap<Tool::BasicTool, qreal> default_widths;
    QDoubleSpinBox *width_box;
    QPushButton *brush_color_button;
    QCheckBox *fill_checkbox;

public:
    DrawToolDetails(Tool::BasicTool basic_tool, QWidget *parent = NULL, const DrawTool *oldtool = NULL);
    ~DrawToolDetails() {}
    QBrush brush() const;
    qreal width() const {return width_box->value();}
public slots:
    void setBrushColor();
};

/**
 * @brief PointingToolDetails: details for pointing tools in a ToolDialog.
 */
class PointingToolDetails : public QWidget
{
    Q_OBJECT

    static const QMap<Tool::BasicTool, qreal> default_sizes;
    QDoubleSpinBox *radius_box;
    QDoubleSpinBox *scale_box = NULL;

public:
    PointingToolDetails(Tool::BasicTool basic_tool, QWidget *parent = NULL, const PointingTool *oldtool = NULL);
    ~PointingToolDetails() {}
    float scale() const {return scale_box ? scale_box->value() : -1.;}
    qreal radius() const {return radius_box->value();}
};

/**
 * @brief TextToolDetails: details for text tool in a ToolDialog.
 */
class TextToolDetails : public QWidget
{
    Q_OBJECT

    QPushButton *font_button;

public:
    TextToolDetails(QWidget *parent = NULL, const TextTool *oldtool = NULL);
    ~TextToolDetails() {}
    QFont font() const {return font_button->font();}
public slots:
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
