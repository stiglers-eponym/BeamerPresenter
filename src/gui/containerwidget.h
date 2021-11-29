#ifndef CONTAINERWIDGET_H
#define CONTAINERWIDGET_H

#include <QWidget>
#include <QLayout>

/**
 * @brief  Widget for arangement of child widgets in QBoxLayout.
 *
 * Flexible container for widget arangement as read from json configuration file.
 *
 * @see TabWidget
 * @see StackedWidget
 * @see FlexLayout
 */
class ContainerWidget : public QWidget
{
    Q_OBJECT

public:
    /// Constructor: initialize QWidget and GuiWidget.
    explicit ContainerWidget(QWidget *parent = NULL) : QWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override
    {return layout() ? layout()->sizeHint() : QSize();}

    /// ContainerWidgets generally have a preferred aspect ratio. Thus hasHeightForWith()==true.
    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // CONTAINERWIDGET_H
