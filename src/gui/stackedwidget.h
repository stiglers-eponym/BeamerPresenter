#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>

/**
 * @brief QStackedWidget with adjusted size hint
 */
class StackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    StackedWidget(QWidget *parent = NULL) noexcept : QStackedWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    /// Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    /// height depends on width (required by FlexLayout).
    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // STACKEDWIDGET_H
