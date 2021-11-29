#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include <QMap>
#include "src/drawing/tool.h"

class QPushButton;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

/**
 * @brief ToolDialog: select tool using GUI
 */
class ToolDialog : public QDialog
{
    Q_OBJECT

    /// select basic tool
    QComboBox *tool_box;
    /// select color (opens QColorDialog)
    QPushButton *color_button = NULL;
    /// select size (or width) of tool
    QDoubleSpinBox *size_box;
    /// select scale (only for magnifier)
    QDoubleSpinBox *scale_box = NULL;
    /// select font (only for TextTool, opens QFontDialog)
    QPushButton *font_button = NULL;
    /// list of checkboxes for input devices
    QMap<int, QCheckBox*> device_buttons;

public:
    /// Constructor: initialize general tool selector.
    ToolDialog(QWidget *parent = NULL);

    /// Adjust current settings to values of tool.
    void setDefault(const Tool *tool);

    /// Create and return a new tool based on current settings.
    Tool *createTool() const;

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicTool(const Tool::BasicTool tool);

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

    /// Select font from a font dialog>
    void selectFont();
};

#endif // TOOLDIALOG_H
