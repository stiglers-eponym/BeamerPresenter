#ifndef TOOLDIALOG_H
#define TOOLDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include "src/names.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"

/**
 * @brief ToolDialog: select tool using GUI
 */
class ToolDialog : public QDialog
{
    Q_OBJECT

    QComboBox *tool_box;
    QPushButton *color_button;
    QDoubleSpinBox *size_box;
    QMap<int, QCheckBox*> device_buttons;

public:
    ToolDialog(QWidget *parent = NULL);

    /// Adjust current settings to values of tool.
    void setDefault(const Tool *tool);

    /// Create and return a new tool based on current settings.
    Tool *createTool() const;

    /// Adjust selection possibilities according to basic tool.
    void adaptToBasicTool(const QString &text);

    /// Set color button color.
    void setColor();

    /// Select a tool. Return NULL if basic_tool is invalid.
    static Tool *selectTool(const Tool *oldtool = NULL);
};

#endif // TOOLDIALOG_H
