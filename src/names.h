#ifndef NAMES_H
#define NAMES_H

#include <QString>
#include <QMap>
#include "src/enumerates.h"
#include "src/gui/guiwidget.h"

/// Convert strings to GuiWidget::WidgetType
static const QMap<QString, GuiWidget::WidgetType> string_to_widget_type
{
    {"container", GuiWidget::ContainerWidget},
    {"stacked", GuiWidget::StackedWidget},
    {"slide", GuiWidget::Slide},
    {"overview", GuiWidget::Overview},
    {"toc", GuiWidget::TOC},
    {"table of contents", GuiWidget::TOC},
    {"notes", GuiWidget::Notes},
    {"button", GuiWidget::Button},
    {"tool selector", GuiWidget::ToolSelector},
    {"settings", GuiWidget::Settings},
    {"clock", GuiWidget::Clock},
    {"timer", GuiWidget::Timer},
    {"slide number", GuiWidget::SlideNumber},
};

/// Translate strings appearing in config to Actions
static const QMap<QString, Action> string_to_action_map
{
    {"previous", PreviousPage},
    {"next", NextPage},
    {"first", FirstPage},
    {"last", LastPage},
    {"update", Update},
    {"next skipping overlays", NextSkippingOverlays},
    {"previous skipping overlays", PreviousSkippingOverlays},
    {"reload", ReloadFiles},
    {"undo", UndoDrawing},
    {"redo", RedoDrawing},
    {"clear", ClearDrawing},
    {"quit", Quit},
};

#endif // NAMES_H
