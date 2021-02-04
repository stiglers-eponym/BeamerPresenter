#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "src/names.h"
#include "src/preferences.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"

/**
 * @brief ToolDialog: select tool using GUI
 */
class ToolDialog : public QDialog
{
    Q_OBJECT

    QComboBox *tool_box;
    QPushButton *color_button = NULL;
    QDoubleSpinBox *size_box;
    QDoubleSpinBox *scale_box = NULL;
    QPushButton *font_button = NULL;
    QMap<int, QCheckBox*> device_buttons;

public:
    ToolDialog(QWidget *parent = NULL);

    /// Adjust current settings to values of tool.
    void setDefault(const Tool *tool);

    /// Create and return a new tool based on current settings.
    Tool *createTool() const;

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicTool(const BasicTool tool);

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicToolStr(const QString &text)
    {adaptToBasicTool(string_to_tool.value(text));}

    /// Select a tool. Return NULL if basic_tool is invalid.
    static Tool *selectTool(const Tool *oldtool = NULL);

public slots:
    /// Set color button color from a color dialog.
    void setColor();

    /// Select font from a font dialog>
    void selectFont();
};

#endif // TOOLDIALOG_H
