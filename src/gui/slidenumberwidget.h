#ifndef SLIDENUMBERWIDGET_H
#define SLIDENUMBERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QResizeEvent>
#include "src/preferences.h"

/// Widget showing current page number (editable) and total number of pages.
/// TODO: keyboard shortcuts while in QLineEdit.
class SlideNumberWidget : public QWidget
{
    Q_OBJECT

    /// Show total number of pages here.
    QLabel *total;

    /// Show and edit current page number here.
    QLineEdit *edit;

public:
    /// Constructor: construct and connect everything.
    explicit SlideNumberWidget(QWidget *parent = NULL);

    /// Trivial destructor: Qt should delete everything automatically.
    ~SlideNumberWidget() {}

    /// This should usually be a good estimate.
    QSize sizeHint() const noexcept override
    {return {150, 25};}

    /// Currently necessary because FlexLayout is kind of broken.
    bool hasHeightForWidth() const noexcept override
    {return true;}

protected:
    /// Resize: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

public slots:
    /// Update current slide and total number of slides.
    void updateText(const int page) noexcept;

    /// Read from edit and send navigation event.
    void readText() noexcept;

signals:
    /// Slide changed, send new page.
    void navigationSignal(const int page);
};

#endif // SLIDENUMBERWIDGET_H
