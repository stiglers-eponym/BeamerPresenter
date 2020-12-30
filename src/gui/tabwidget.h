#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabBar>
#include <QTabWidget>

class TabWidget : public QTabWidget
{
    Q_OBJECT

public:
    /// Constructor: set size policy.
    TabWidget(QWidget *parent = nullptr) noexcept : QTabWidget(parent)
    {setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);}

    // Return sizeHint based on layout.
    QSize sizeHint() const noexcept override;

    bool hasHeightForWidth() const noexcept override
    {return true;}
};

#endif // TABWIDGET_H
