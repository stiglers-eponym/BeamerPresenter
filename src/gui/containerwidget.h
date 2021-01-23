#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QDebug>
#include "src/gui/flexlayout.h"

/// This whole construction might change in the future.
/// Widget for arangement of child widgets in QBoxLayout as read from
/// json configuration file.
class ContainerWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor: initialize QWidget and GuiWidget.
    explicit ContainerWidget(QWidget *parent = NULL) : QWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    // Return sizeHint based on layout.
    QSize sizeHint() const noexcept override
    {return layout() ? layout()->sizeHint() : QSize();}

    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // CONTAINERWIDGET_H
