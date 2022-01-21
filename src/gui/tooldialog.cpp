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
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>

DrawToolDetails::DrawToolDetails(QWidget *parent, const DrawTool *oldtool) :
    QWidget(parent),
    width_box(new QDoubleSpinBox(this)),
    brush_color_button(new QPushButton(this)),
    fill_checkbox(new QCheckBox(this))
{
    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(tr("stroke width (in pt)"), width_box);
    if (oldtool) {
        QPalette palette = brush_color_button->palette();
        palette.setBrush(QPalette::Button, oldtool->brush());
        brush_color_button->setPalette(palette);
        brush_color_button->setText(palette.color(QPalette::Button).name());
        fill_checkbox->setCheckState(oldtool->brush().style() == Qt::NoBrush ? Qt::Unchecked : Qt::Checked);
    }
    else
        brush_color_button->setText(tr("color"));
    layout->addRow(tr("fill color"), brush_color_button);
    layout->addRow(tr("fill"), fill_checkbox);
    connect(brush_color_button, &QPushButton::clicked, this, &DrawToolDetails::setBrushColor);
    setLayout(layout);
}

void DrawToolDetails::setBrushColor()
{
    QPalette button_palette = brush_color_button->palette();
    const QColor color = QColorDialog::getColor(button_palette.button().color(), this, tr("Fill color"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;
    QBrush brush(color);
    if (fill_checkbox->isChecked())
        brush.setStyle(Qt::SolidPattern);
    else
        brush.setStyle(Qt::NoBrush);
    button_palette.setColor(QPalette::Button, color);
    brush_color_button->setPalette(button_palette);
    brush_color_button->setText(color.name());
}

QBrush DrawToolDetails::brush() const
{
    QBrush brush(brush_color_button->palette().button());
    if (!fill_checkbox->isChecked())
        brush.setStyle(Qt::NoBrush);
    return brush;
}

const QMap<Tool::BasicTool, qreal> PointingToolDetails::default_sizes = {
        {Tool::Pointer, 8.},
        {Tool::Eraser, 20.},
        {Tool::Torch, 80.},
        {Tool::Magnifier, 100.},
    };

PointingToolDetails::PointingToolDetails(Tool::BasicTool basic_tool, QWidget *parent, const PointingTool *oldtool) :
    QWidget(parent),
    radius_box(new QDoubleSpinBox(this))
{
    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(tr("radius (in pt)"), radius_box);
    if (oldtool && oldtool->tool() == basic_tool)
        radius_box->setValue(oldtool->size());
    else
        radius_box->setValue(default_sizes.value(basic_tool, 0.));
    if (basic_tool == Tool::Magnifier)
    {
        scale_box = new QDoubleSpinBox(this);
        layout->addRow(tr("Magnification scale"), scale_box);
        if (oldtool && oldtool->tool() == Tool::Magnifier)
        scale_box->setValue((oldtool && oldtool->tool() == Tool::Magnifier) ? oldtool->scale() : 2.);
    }
    else if (basic_tool == Tool::Eraser)
    {
        scale_box = new QDoubleSpinBox(this);
        layout->addRow(tr("Eraser border width"), scale_box);
        scale_box->setValue((oldtool && oldtool->tool() == Tool::Eraser) ? oldtool->scale() : 0.5);
    }
    setLayout(layout);
}

TextToolDetails::TextToolDetails(QWidget *parent, const TextTool *oldtool) :
    QWidget(parent),
    font_button(new QPushButton("font", this))
{
    QFormLayout *layout = new QFormLayout(this);
    layout->addRow(tr("font"), font_button);
    setLayout(layout);
    if (oldtool)
    {
        font_button->setText(oldtool->font().toString());
        font_button->setFont(oldtool->font());
    }
    connect(font_button, &QPushButton::clicked, this, &TextToolDetails::selectFont);
}

void TextToolDetails::selectFont()
{
    bool ok;
    QFont newfont = QFontDialog::getFont(&ok, font_button->font(), this, tr("Font for Text input"));
    font_button->setText(newfont.toString());
    font_button->setFont(newfont);
}


ToolDialog::ToolDialog(QWidget *parent) :
    QDialog(parent),
    tool_box(new QComboBox(this)),
    color_button(new QPushButton(this))
{
    QFormLayout *layout = new QFormLayout();

    // Basic tool
    for (auto it = string_to_tool.cbegin(); it != string_to_tool.cend(); ++it)
        tool_box->addItem(it.key());
    connect(tool_box, &QComboBox::currentTextChanged, this, &ToolDialog::adaptToBasicToolStr);
    tool_box->setCurrentText("invalid");
    layout->addRow(tr("Tool:"), tool_box);

    // Color
    color_button->setText(tr("color"));
    connect(color_button, &QPushButton::clicked, this, &ToolDialog::setColor);
    layout->addRow(tr("Color:"), color_button);

    // Input devices
    {
        QGroupBox *device_group = new QGroupBox(tr("Input devices"), this);
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

    // Exit
    QPushButton *exit_button = new QPushButton(tr("return"), this);
    connect(exit_button, &QPushButton::clicked, this, &ToolDialog::accept);
    layout->addWidget(exit_button);

    setLayout(layout);
}

void ToolDialog::adaptToBasicTool(const Tool::BasicTool basic_tool)
{
    delete tool_specific;
    tool_specific = NULL;
    switch (basic_tool)
    {
    case Tool::TextInputTool:
        tool_specific = new TextToolDetails(this);
        break;
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen:
        tool_specific = new DrawToolDetails(this);
        break;
    case Tool::Torch:
    case Tool::Pointer:
    case Tool::Eraser:
    case Tool::Magnifier:
        tool_specific = new PointingToolDetails(basic_tool, this);
        break;
    default:
        break;
    }
    if (tool_specific)
        layout()->addWidget(tool_specific);
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

    delete tool_specific;
    tool_specific = NULL;
    switch (tool->tool())
    {
    case Tool::TextInputTool:
    {
        const TextTool *text_tool = static_cast<const TextTool*>(tool);
        tool_specific = new TextToolDetails(this, text_tool);
        color = text_tool->color();
        break;
    }
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen:
    {
        const DrawTool *draw_tool = static_cast<const DrawTool*>(tool);
        tool_specific = new DrawToolDetails(this, draw_tool);
        color = draw_tool->color();
        break;
    }
    case Tool::Magnifier:
    case Tool::Eraser:
    case Tool::Torch:
    case Tool::Pointer:
    {
        const PointingTool *pointing_tool = static_cast<const PointingTool*>(tool);
        tool_specific = new PointingToolDetails(tool->tool(), this, pointing_tool);
        color = pointing_tool->color();
        break;
    }
    default:
        break;
    }
    if (tool_specific)
        layout()->addWidget(tool_specific);

    color_button->setText(color.name());
    button_palette.setColor(QPalette::Button, color);
    color_button->setPalette(button_palette);
}

Tool *ToolDialog::createTool() const
{
    const Tool::BasicTool basic_tool = string_to_tool.value(tool_box->currentText());
    debug_verbose(DebugDrawing, "Dialog selected basic tool" << basic_tool);
    if (basic_tool == Tool::InvalidTool)
        return NULL;
    int device = 0;
    for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
        if ((*it)->isChecked())
            device |= it.key();
    const QColor color = color_button->palette().color(QPalette::Button);

    switch (basic_tool)
    {
    case Tool::TextInputTool:
    {
        TextToolDetails *details = static_cast<TextToolDetails*>(tool_specific);
        return new TextTool(details->font(), color, device);
    }
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen:
    {
        DrawToolDetails *details = static_cast<DrawToolDetails*>(tool_specific);
        return new DrawTool(
                    basic_tool,
                    device,
                    QPen(color, details->width(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin),
                    details->brush(),
                    basic_tool == Tool::Highlighter ? QPainter::CompositionMode_Darken : QPainter::CompositionMode_SourceOver
                );
    }
    case Tool::Torch:
    {
        PointingToolDetails *details = static_cast<PointingToolDetails*>(tool_specific);
        PointingTool *tool = new PointingTool(basic_tool, details->radius(), color, device);
        return tool;
    }
    case Tool::Magnifier:
    case Tool::Eraser:
    {
        PointingToolDetails *details = static_cast<PointingToolDetails*>(tool_specific);
        PointingTool *tool = new PointingTool(basic_tool, details->radius(), color, device);
        tool->setScale(details->scale());
        return tool;
    }
    case Tool::Pointer:
    {
        PointingToolDetails *details = static_cast<PointingToolDetails*>(tool_specific);
        PointingTool *tool = new PointingTool(basic_tool, details->radius(), color, device);
        tool->initPointerBrush();
        return tool;
    }
    default:
        return new Tool(basic_tool, device);
    }
}

void ToolDialog::setColor()
{
    QPalette button_palette = color_button->palette();
    const QColor color = QColorDialog::getColor(button_palette.button().color(), this, tr("Tool color"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;
    button_palette.setColor(QPalette::Button, color);
    color_button->setPalette(button_palette);
    color_button->setText(color.name());
}
