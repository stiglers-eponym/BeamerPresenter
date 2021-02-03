#include "tooldialog.h"

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
    connect(tool_box, &QComboBox::currentTextChanged, this, &ToolDialog::adaptToBasicTool);
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
    connect(exit_button, &QPushButton::clicked, this, &ToolDialog::close);
    layout->addWidget(exit_button);

    setLayout(layout);
}

void ToolDialog::adaptToBasicTool(const QString &text)
{
    switch (string_to_tool.value(text))
    {
    case Eraser:
    case Magnifier:
        color_button->hide();
        break;
    case Pen:
    case Highlighter:
    case Torch:
    case Pointer:
    case FixedWidthPen:
        color_button->show();
        break;
    case NoTool:
    case InvalidTool:
        color_button->hide();
        break;
    }
}

Tool *ToolDialog::selectTool(const Tool *oldtool)
{
    ToolDialog dialog;
    if (oldtool)
        dialog.setDefault(oldtool);
    dialog.exec();
    return dialog.createTool();
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
    if (tool->tool() & AnyDrawTool)
    {
        const DrawTool *draw_tool = static_cast<const DrawTool*>(tool);
        size_box->setValue(draw_tool->width());
        color = draw_tool->color();
    }
    else if (tool->tool() & AnyPointingTool)
    {
        const PointingTool *pointing_tool = static_cast<const PointingTool*>(tool);
        size_box->setValue(pointing_tool->size());
        color = pointing_tool->color();
    }
    color_button->setText(color.name());
    button_palette.setColor(QPalette::Button, color);
    color_button->setPalette(button_palette);
}

Tool *ToolDialog::createTool() const
{
    const BasicTool basic_tool = string_to_tool.value(tool_box->currentText());
    int device = 0;
    for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
        if ((*it)->isChecked())
            device |= it.key();
    const QColor color = color_button->palette().color(QPalette::Button);
    if (basic_tool & AnyDrawTool)
        return new DrawTool(basic_tool, device, QPen(color, size_box->value(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else if (basic_tool & AnyPointingTool)
        return new PointingTool(basic_tool, size_box->value(), color, device);
    else if (basic_tool != InvalidTool)
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
