// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/gui/tooldialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>

#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/selectiontool.h"
#include "src/drawing/texttool.h"
#include "src/enumerates.h"
#include "src/log.h"

DrawToolDetails::DrawToolDetails(Tool::BasicTool basic_tool, QWidget *parent,
                                 std::shared_ptr<const DrawTool> oldtool)
    : QWidget(parent),
      width_box(new QDoubleSpinBox(this)),
      brush_color_button(new QPushButton(this)),
      fill_checkbox(new QCheckBox(this)),
      shape_box(new QComboBox(this)),
      pen_style_box(new QComboBox(this)),
      brush_style_box(new QComboBox(this))
{
  QFormLayout *layout = new QFormLayout(this);

  // Shape selection
  for (auto it = shape_codes.cbegin(); it != shape_codes.cend(); ++it)
    shape_box->addItem(DrawTool::tr(it->c_str()),
                       QVariant::fromValue(it.key()));
  layout->addRow(tr("shape"), shape_box);
  shape_box->setCurrentIndex(shape_box->findData(
      QVariant::fromValue(oldtool ? oldtool->shape() : DrawTool::Freehand)));
#if (QT_VERSION_MAJOR >= 6)
  connect(shape_box, &QComboBox::currentIndexChanged, this,
          &DrawToolDetails::changeShape);
#else
  connect(shape_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &DrawToolDetails::changeShape);
#endif

  // Width selection
  layout->addRow(tr("stroke width (in pt)"), width_box);
  if (oldtool && oldtool->tool() == basic_tool)
    width_box->setValue(oldtool->width());
  else
    switch (basic_tool) {
      case Tool::Pen:
      case Tool::FixedWidthPen:
        width_box->setValue(2.);
        break;
      case Tool::Highlighter:
        width_box->setValue(10.);
        break;
      default:
        width_box->setValue(0.);
        break;
    }

  // Pen style selection
  pen_style_box->addItem(tr("no pen"), QVariant::fromValue(Qt::NoPen));
  pen_style_box->addItem(tr("solid"), QVariant::fromValue(Qt::SolidLine));
  pen_style_box->addItem(tr("dashed"), QVariant::fromValue(Qt::DashLine));
  pen_style_box->addItem(tr("dotted"), QVariant::fromValue(Qt::DotLine));
  pen_style_box->addItem(tr("dash-dotted"),
                         QVariant::fromValue(Qt::DashDotLine));
  pen_style_box->addItem(tr("dash-dot-dot"),
                         QVariant::fromValue(Qt::DashDotDotLine));
  layout->addRow(tr("pen style"), pen_style_box);
  pen_style_box->setCurrentIndex(pen_style_box->findData(QVariant::fromValue(
      oldtool ? oldtool->pen().style() : Qt::PenStyle::SolidLine)));

  // Fill checkbox
  layout->addRow(tr("fill"), fill_checkbox);
  fill_checkbox->setCheckState(
      oldtool && oldtool->brush().style() != Qt::NoBrush ? Qt::Checked
                                                         : Qt::Unchecked);

  // Brush color selection
  if (oldtool) {
    QColor color = oldtool->brush().color();
    if (!color.isValid()) color = oldtool->color();
    brush_color_button->setStyleSheet(
        "background-color:" + color.name(QColor::HexArgb) + ";");
    QPalette palette = brush_color_button->palette();
    if (oldtool->brush().style() == Qt::NoBrush)
      palette.setColor(QPalette::Button, color);
    else
      palette.setBrush(QPalette::Button, oldtool->brush());
    brush_color_button->setPalette(palette);
    brush_color_button->setText(color.name(QColor::HexArgb));
  } else
    brush_color_button->setText(tr("select color"));
  layout->addRow(tr("fill color"), brush_color_button);
  connect(brush_color_button, &QPushButton::clicked, this,
          &DrawToolDetails::setBrushColor);

  // Brush style selection
  brush_style_box->addItem(tr("no brush"), QVariant::fromValue(Qt::NoBrush));
  brush_style_box->addItem(tr("solid pattern"),
                           QVariant::fromValue(Qt::SolidPattern));
  brush_style_box->addItem(tr("dense pattern 1"),
                           QVariant::fromValue(Qt::Dense1Pattern));
  brush_style_box->addItem(tr("dense pattern 2"),
                           QVariant::fromValue(Qt::Dense2Pattern));
  brush_style_box->addItem(tr("dense pattern 3"),
                           QVariant::fromValue(Qt::Dense3Pattern));
  brush_style_box->addItem(tr("dense pattern 4"),
                           QVariant::fromValue(Qt::Dense4Pattern));
  brush_style_box->addItem(tr("dense pattern 5"),
                           QVariant::fromValue(Qt::Dense5Pattern));
  brush_style_box->addItem(tr("dense pattern 6"),
                           QVariant::fromValue(Qt::Dense6Pattern));
  brush_style_box->addItem(tr("dense pattern 7"),
                           QVariant::fromValue(Qt::Dense7Pattern));
  brush_style_box->addItem(tr("horizontally hatched"),
                           QVariant::fromValue(Qt::HorPattern));
  brush_style_box->addItem(tr("vertically hatched"),
                           QVariant::fromValue(Qt::VerPattern));
  brush_style_box->addItem(tr("cross pattern"),
                           QVariant::fromValue(Qt::CrossPattern));
  brush_style_box->addItem(tr("hatched towards top right"),
                           QVariant::fromValue(Qt::BDiagPattern));
  brush_style_box->addItem(tr("hatched towards top left"),
                           QVariant::fromValue(Qt::FDiagPattern));
  brush_style_box->addItem(tr("diagonal cross pattern"),
                           QVariant::fromValue(Qt::DiagCrossPattern));
  layout->addRow(tr("brush style"), brush_style_box);
  brush_style_box->setCurrentIndex(
      brush_style_box->findData(QVariant::fromValue(
          oldtool ? oldtool->brush().style() : Qt::SolidPattern)));
#if (QT_VERSION_MAJOR >= 6)
  connect(brush_style_box, &QComboBox::currentIndexChanged, this,
          &DrawToolDetails::setBrushStyle);
#else
  connect(brush_style_box, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &DrawToolDetails::setBrushStyle);
#endif

  changeShape(shape_box->currentIndex());
  setLayout(layout);
}

void DrawToolDetails::setBrushColor()
{
  QPalette button_palette = brush_color_button->palette();
  const QColor color =
      QColorDialog::getColor(button_palette.button().color(), this,
                             tr("Fill color"), QColorDialog::ShowAlphaChannel);
  if (!color.isValid()) return;
  brush_color_button->setStyleSheet(
      "background-color:" + color.name(QColor::HexArgb) + ";");
  QBrush brush = button_palette.brush(QPalette::Button);
  brush.setColor(color);
  fill_checkbox->setCheckState(Qt::Checked);
  button_palette.setBrush(QPalette::Button, brush);
  brush_color_button->setPalette(button_palette);
  brush_color_button->setText(color.name(QColor::HexArgb));
  if (brush_style_box->currentData().value<Qt::BrushStyle>() == Qt::NoBrush)
    brush_style_box->setCurrentIndex(
        brush_style_box->findData(QVariant::fromValue(Qt::SolidPattern)));
}

QBrush DrawToolDetails::brush() const
{
  QBrush brush(brush_color_button->palette().button());
  if (!fill_checkbox->isChecked()) brush.setStyle(Qt::NoBrush);
  return brush;
}

void DrawToolDetails::changeShape(int index)
{
  const DrawTool::Shape newshape =
      shape_box->itemData(index).value<DrawTool::Shape>();
  const bool disable =
      newshape == DrawTool::Arrow || newshape == DrawTool::Line;
  brush_color_button->setDisabled(disable);
  brush_style_box->setDisabled(disable);
  if (disable) fill_checkbox->setChecked(false);
  fill_checkbox->setDisabled(disable);
}

void DrawToolDetails::setBrushStyle(int index)
{
  QPalette button_palette = brush_color_button->palette();
  QBrush brush = button_palette.brush(QPalette::Button);
  brush.setStyle(brush_style_box->itemData(index).value<Qt::BrushStyle>());
  button_palette.setBrush(QPalette::Button, brush);
  brush_color_button->setPalette(button_palette);
  fill_checkbox->setCheckState(brush.style() == Qt::NoBrush ? Qt::Unchecked
                                                            : Qt::Checked);
}

PointingToolDetails::PointingToolDetails(
    Tool::BasicTool basic_tool, QWidget *parent,
    std::shared_ptr<const PointingTool> oldtool)
    : QWidget(parent), radius_box(new QDoubleSpinBox(this))
{
  QFormLayout *layout = new QFormLayout(this);
  layout->addRow(tr("radius (in pt)"), radius_box);
  if (oldtool && oldtool->tool() == basic_tool)
    radius_box->setValue(oldtool->size());
  else
    switch (basic_tool) {
      case Tool::Pointer:
        radius_box->setValue(8.);
        break;
      case Tool::Eraser:
        radius_box->setValue(20.);
        break;
      case Tool::Torch:
        radius_box->setValue(80.);
        break;
      case Tool::Magnifier:
        radius_box->setValue(100.);
        break;
      default:
        radius_box->setValue(0.);
        break;
    }
  if (basic_tool == Tool::Magnifier) {
    scale_box = new QDoubleSpinBox(this);
    scale_box->setMinimum(0.1);
    scale_box->setMaximum(5.);
    layout->addRow(tr("Magnification scale"), scale_box);
    scale_box->setValue((oldtool && oldtool->tool() == Tool::Magnifier)
                            ? oldtool->scale()
                            : 2.);
  } else if (basic_tool == Tool::Eraser) {
    scale_box = new QDoubleSpinBox(this);
    scale_box->setMinimum(0.);
    scale_box->setMaximum(25.);
    layout->addRow(tr("Eraser border width"), scale_box);
    scale_box->setValue(
        (oldtool && oldtool->tool() == Tool::Eraser) ? oldtool->scale() : 0.5);
  }
  setLayout(layout);
}

TextToolDetails::TextToolDetails(QWidget *parent,
                                 std::shared_ptr<const TextTool> oldtool)
    : QWidget(parent), font_button(new QPushButton("font", this))
{
  QFormLayout *layout = new QFormLayout(this);
  layout->addRow(tr("font"), font_button);
  setLayout(layout);
  if (oldtool) {
    font_button->setText(oldtool->font().toString());
    font_button->setFont(oldtool->font());
  }
  connect(font_button, &QPushButton::clicked, this,
          &TextToolDetails::selectFont);
}

void TextToolDetails::selectFont()
{
  bool ok;
  QFont newfont = QFontDialog::getFont(&ok, font_button->font(), this,
                                       tr("Font for Text input"));
  font_button->setText(newfont.toString());
  font_button->setFont(newfont);
}

ToolDialog::ToolDialog(QWidget *parent)
    : QDialog(parent),
      tool_box(new QComboBox(this)),
      color_button(new QPushButton(this))
{
  QFormLayout *layout = new QFormLayout();

  // Basic tool
  for (auto it = string_to_tool.cbegin(); it != string_to_tool.cend(); ++it)
    tool_box->addItem(Tool::tr(it.key().toLatin1().constData()),
                      QVariant::fromValue(*it));
#if (QT_VERSION_MAJOR >= 6)
  connect(tool_box, &QComboBox::currentIndexChanged, this,
          &ToolDialog::adaptToBasicToolIdx);
#else
  connect(tool_box, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ToolDialog::adaptToBasicToolIdx);
#endif
  tool_box->setCurrentIndex(
      tool_box->findData(QVariant::fromValue(DrawTool::InvalidTool)));
  layout->addRow(tr("Tool:"), tool_box);

  // Color
  color_button->setText(tr("color"));
  connect(color_button, &QPushButton::clicked, this, &ToolDialog::setColor);
  layout->addRow(tr("Color:"), color_button);

  // Input devices
  {
    QGroupBox *device_group = new QGroupBox(tr("Input devices"), this);
    QGridLayout *device_layout = new QGridLayout();
    device_layout->setSpacing(0);
    int i = 0;
    for (auto it = string_to_input_device.cbegin();
         it != string_to_input_device.cend(); ++it) {
      QCheckBox *button =
          new QCheckBox(Tool::tr(it.key().c_str()), device_group);
      device_layout->addWidget(button, i / 2, i % 2);
      device_buttons.insert(*it, button);
      ++i;
    }
    device_group->setLayout(device_layout);
    layout->addWidget(device_group);
  }

  // Exit
  QPushButton *exit_button = new QPushButton(tr("return"), this);
  connect(exit_button, &QPushButton::clicked, this, &ToolDialog::accept);
  layout->insertRow(4, exit_button);

  setLayout(layout);
}

void ToolDialog::adaptToBasicTool(const Tool::BasicTool basic_tool)
{
  delete tool_specific;
  tool_specific = nullptr;
  switch (basic_tool) {
    case Tool::TextInputTool:
      tool_specific = new TextToolDetails(this);
      break;
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen:
      tool_specific = new DrawToolDetails(basic_tool, this);
      break;
    case Tool::Torch:
    case Tool::Pointer:
    case Tool::Eraser:
    case Tool::Magnifier:
      tool_specific = new PointingToolDetails(basic_tool, this);
      break;
    default:
      return;
  }
  if (tool_specific)
    static_cast<QFormLayout *>(layout())->insertRow(3, tool_specific);
}

std::shared_ptr<Tool> ToolDialog::selectTool(
    std::shared_ptr<const Tool> oldtool)
{
  ToolDialog dialog;
  if (oldtool) dialog.setDefault(oldtool);
  if (dialog.exec() == QDialog::Accepted)
    return dialog.createTool();
  else
    return nullptr;
}

void ToolDialog::setDefault(std::shared_ptr<const Tool> tool)
{
  if (!tool) return;
  tool_box->setCurrentIndex(
      tool_box->findData(QVariant::fromValue(tool->tool())));
  for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
    (*it)->setChecked((it.key() & tool->device()) == it.key());
  QPalette button_palette = color_button->palette();
  QColor color;

  delete tool_specific;
  tool_specific = nullptr;
  switch (tool->tool()) {
    case Tool::TextInputTool: {
      const auto text_tool = std::static_pointer_cast<const TextTool>(tool);
      tool_specific = new TextToolDetails(this, text_tool);
      color = text_tool->color();
      break;
    }
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen: {
      const auto draw_tool = std::static_pointer_cast<const DrawTool>(tool);
      tool_specific = new DrawToolDetails(tool->tool(), this, draw_tool);
      color = draw_tool->color();
      break;
    }
    case Tool::Magnifier:
    case Tool::Eraser:
    case Tool::Torch:
    case Tool::Pointer: {
      const auto pointing_tool =
          std::static_pointer_cast<const PointingTool>(tool);
      tool_specific =
          new PointingToolDetails(tool->tool(), this, pointing_tool);
      color = pointing_tool->color();
      break;
    }
    default:
      break;
  }
  if (tool_specific)
    static_cast<QFormLayout *>(layout())->insertRow(3, tool_specific);

  color_button->setText(color.name(QColor::HexArgb));
  button_palette.setColor(QPalette::Button, color);
  color_button->setPalette(button_palette);
  color_button->setStyleSheet(
      "background-color:" + color.name(QColor::HexArgb) + ";");
}

std::shared_ptr<Tool> ToolDialog::createTool() const
{
  const Tool::BasicTool basic_tool =
      tool_box->currentData().value<Tool::BasicTool>();
  debug_verbose(DebugDrawing, "Dialog selected basic tool" << basic_tool);
  if (basic_tool == Tool::InvalidTool) return nullptr;
  int device = 0;
  for (auto it = device_buttons.cbegin(); it != device_buttons.cend(); ++it)
    if ((*it)->isChecked()) device |= it.key();
  const QColor color = color_button->palette().color(QPalette::Button);

  switch (basic_tool) {
    case Tool::TextInputTool:
      return std::shared_ptr<Tool>(
          new TextTool(static_cast<TextToolDetails *>(tool_specific)->font(),
                       color, device));
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::FixedWidthPen: {
      DrawToolDetails *details = static_cast<DrawToolDetails *>(tool_specific);
      return std::shared_ptr<Tool>(
          new DrawTool(basic_tool, device,
                       QPen(color, details->width(), details->penStyle(),
                            Qt::RoundCap, Qt::RoundJoin),
                       details->brush(),
                       basic_tool == Tool::Highlighter
                           ? QPainter::CompositionMode_Darken
                           : QPainter::CompositionMode_SourceOver,
                       details->shape()));
    }
    case Tool::Torch: {
      PointingToolDetails *details =
          static_cast<PointingToolDetails *>(tool_specific);
      return std::shared_ptr<PointingTool>(
          new PointingTool(basic_tool, details->radius(), color, device));
    }
    case Tool::Magnifier:
    case Tool::Eraser: {
      PointingToolDetails *details =
          static_cast<PointingToolDetails *>(tool_specific);
      const auto tool = std::shared_ptr<PointingTool>(
          new PointingTool(basic_tool, details->radius(), color, device));
      tool->setScale(details->scale());
      return tool;
    }
    case Tool::Pointer: {
      PointingToolDetails *details =
          static_cast<PointingToolDetails *>(tool_specific);
      const auto tool = std::shared_ptr<PointingTool>(
          new PointingTool(basic_tool, details->radius(), color, device));
      tool->initPointerBrush();
      return tool;
    }
    case Tool::BasicSelectionTool:
    case Tool::RectSelectionTool:
    case Tool::FreehandSelectionTool:
      return std::shared_ptr<SelectionTool>(
          new SelectionTool(basic_tool, device));
    default:
      return std::shared_ptr<Tool>(new Tool(basic_tool, device));
  }
}

void ToolDialog::setColor()
{
  QPalette button_palette = color_button->palette();
  const QColor color =
      QColorDialog::getColor(button_palette.button().color(), this,
                             tr("Tool color"), QColorDialog::ShowAlphaChannel);
  if (!color.isValid()) return;
  button_palette.setColor(QPalette::Button, color);
  color_button->setPalette(button_palette);
  color_button->setText(color.name(QColor::HexArgb));
  color_button->setStyleSheet(
      "background-color:" + color.name(QColor::HexArgb) + ";");
}
