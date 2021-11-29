#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

/**
 * @brief QTabWidget with adjusted size hint
 * @see StackedWidget
 * @see ContainerWidget
 */
class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    TabWidget(QWidget *parent = NULL) noexcept : QTabWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // TABWIDGET_H
