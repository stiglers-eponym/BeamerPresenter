#include "src/gui/tooldialog.h"
#include "src/preferences.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/texttool.h"
#include "src/drawing/pointingtool.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>

ToolDialog::ToolDialog(QWidget *parent) :
    QDialog(parent),
    tool_box(new QComboBox(this)),
    color_button(new QPushButton(this)),
    size_box(new QDoubleSpinBox(this))
{
    QFormLayout *layout = new QFormLayout();

    // Basic tool
    for (auto it = string_to_tool.cbegin(); it != string_to_tool.cend(); ++it)
        tool_box->addItem(it.key());
    connect(tool_box, &QComboBox::currentTextChanged, this, &ToolDialog::adaptToBasicToolStr);
    tool_box->setCurrentText("invalid");
    layout->addRow("Tool:", tool_box);

    // Color
    color_button->setText("color");
    connect(color_button, &QPushButton::clicked, this, &ToolDialog::setColor);
    layout->addRow("Color:", color_button);

    // Input devices
    {
        QGroupBox *device_group = new QGroupBox("Input devices", this);
        QVBoxLayout *device_layout = new QVBoxLayout();
        for (auto it = string_to_input_device.cbegin(); it != string_to_input_device.cend(); ++it)
        {
            QCheckBox *button = new QCheckBox(it.key(), device_group);
            device_layout->addWidget(button);
            device_buttons.insert(*it, button);
        }
        device_group->setLayout(device_layout);
        layout->addWidget(device_group);
    }

    // Size
    size_box->setMaximum(999.99);
    layout->addRow("Size:", size_box);

    // Exit
    QPushButton *exit_button = new QPushButton("return", this);
    connect(exit_button, &QPushButton::clicked, this, &ToolDialog::accept);
    layout->addWidget(exit_button);

    setLayout(layout);
}

void ToolDialog::adaptToBasicTool(const Tool::BasicTool tool)
{
    switch (tool)
    {
    case Tool::TextInputTool:
        if (!font_button)
        {
            font_button = new QPushButton("font", this);
            connect(font_button, &QPushButton::clicked, this, &ToolDialog::selectFont);
            static_cast<QFormLayout*>(layout())->addRow("font:", font_button);
        }
        else
            font_button->show();
        if (color_button)
            color_button->show();
        if (size_box)
            size_box->hide();
        if (scale_box)
            scale_box->hide();
        break;
    case Tool::Magnifier:
        if (!scale_box)
        {
            scale_box = new QDoubleSpinBox(this);
            scale_box->setMaximum(5);
            scale_box->setMinimum(.2);
            static_cast<QFormLayout*>(layout())->addRow("scale:", scale_box);
        }
        else
            scale_box->show();
        if (size_box)
            size_box->show();
        if (color_button)
            color_button->hide();
        if (font_button)
            font_button->hide();
        break;
    case Tool::Eraser:
        if (color_button)
            color_button->hide();
        if (scale_box)
            scale_box->hide();
        if (font_button)
            font_button->hide();
        if (size_box)
            size_box->show();
        break;
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::Torch:
    case Tool::Pointer:
    case Tool::FixedWidthPen:
        if (color_button)
            color_button->show();
        if (scale_box)
            scale_box->hide();
        if (font_button)
            font_button->hide();
        if (size_box)
            size_box->show();
        break;
    case Tool::NoTool:
    case Tool::InvalidTool:
        if (color_button)
            color_button->hide();
        if (scale_box)
            scale_box->hide();
        if (font_button)
            font_button->hide();
        if (size_box)
            size_box->hide();
        break;
    default:
        break;
    }
}

Tool *ToolDialog::selectTool(const Tool *oldtool)
{
    ToolDialog dialog;
    if (oldtool)
        dialog.setDefault(oldtool);
    if (dialog.exec() == QDialog::Accepted)
        return dialog.createTool();
    else
        return NULL;
}

void ToolDialog::setDefault(const Tool *tool)
{
    if (!tool)
        return;
    tool_box->setCurrentText(string_to_tool.key(tool->tool()));
    for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
        (*it)->setChecked(it.key() & tool->device());
    QPalette button_palette = color_button->palette();
    QColor color;
    if (tool->tool() & Tool::AnyDrawTool)
    {
        const DrawTool *draw_tool = static_cast<const DrawTool*>(tool);
        size_box->setValue(draw_tool->width());
        color = draw_tool->color();
    }
    else if (tool->tool() & Tool::AnyPointingTool)
    {
        const PointingTool *pointing_tool = static_cast<const PointingTool*>(tool);
        size_box->setValue(pointing_tool->size());
        color = pointing_tool->color();
        if (tool->tool() == Tool::Magnifier && scale_box)
            scale_box->setValue(pointing_tool->scale());
    }
    else if (tool->tool() == Tool::TextInputTool)
    {
        const TextTool *text_tool = static_cast<const TextTool*>(tool);
        if (font_button)
        {
            font_button->setFont(text_tool->font());
            font_button->setText(text_tool->font().toString());
        }
        color = text_tool->color();
    }
    if (color_button)
    {
        color_button->setText(color.name());
        button_palette.setColor(QPalette::Button, color);
        color_button->setPalette(button_palette);
    }
}

Tool *ToolDialog::createTool() const
{
    const Tool::BasicTool basic_tool = string_to_tool.value(tool_box->currentText());
    debug_verbose(DebugDrawing) << "Dialog selected basic tool" << basic_tool;
    if (basic_tool == Tool::InvalidTool)
        return NULL;
    int device = 0;
    for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
        if ((*it)->isChecked())
            device |= it.key();
    const QColor color = color_button->palette().color(QPalette::Button);
    if (basic_tool & Tool::AnyDrawTool)
        return new DrawTool(
                    basic_tool,
                    device,
                    QPen(color, size_box->value(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),
                    basic_tool == Tool::Highlighter ? QPainter::CompositionMode_Darken : QPainter::CompositionMode_SourceOver
                );
    else if (basic_tool & Tool::AnyPointingTool)
    {
        PointingTool *tool = new PointingTool(basic_tool, size_box->value(), color, device);
        if (basic_tool == Tool::Magnifier && scale_box)
            tool->setScale(scale_box->value());
        if (basic_tool == Tool::Pointer)
            tool->initPointerBrush();
        if (basic_tool == Tool::Eraser)
            tool->setScale(0.5);
        return tool;
    }
    else if (basic_tool == Tool::TextInputTool)
        return new TextTool(font_button ? font_button->font() : font(), color, device);
    else if (basic_tool != Tool::InvalidTool)
        return new Tool(basic_tool, device);
    return NULL;
}

void ToolDialog::setColor()
{
    QPalette button_palette = color_button->palette();
    const QColor color = QColorDialog::getColor(button_palette.button().color(), this, "Tool color", QColorDialog::ShowAlphaChannel);
    button_palette.setColor(QPalette::Button, color);
    color_button->setPalette(button_palette);
    color_button->setText(color.name());
}

void ToolDialog::selectFont()
{
    if (!font_button)
        return;
    bool ok;
    QFont newfont = QFontDialog::getFont(&ok, font_button->font(), this, "Font for Text input");
    font_button->setText(newfont.toString());
    font_button->setFont(newfont);
}
