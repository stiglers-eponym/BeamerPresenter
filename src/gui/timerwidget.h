#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>
#include <QTime>
#include "src/enumerates.h"

class QResizeEvent;
class QLineEdit;
class QLabel;
class QTimer;

/// Map time (in ms) left for a slide to color.
/// This is the default colormap for timer.
static const QMap<qint32, QRgb> default_timer_colormap
{
    {-300000, qRgb(255,   0,   0)},
    { -90000, qRgb(255, 255,   0)},
    {      0, qRgb(  0, 255,   0)},
    {  90000, qRgb(  0, 255, 255)},
    { 300000, qRgb(255, 255, 255)},
};

/**
 * @brief Editable timer for presentation and target time.
 *
 * 2 QLineEdit's "passed" and "total" show presentation time passed and
 * estimate or target for total time.
 *
 * Emits timeout event.
 */
class TimerWidget : public QWidget
{
    Q_OBJECT

public:
    enum
    {
        Timeout = 1 << 0,
        SetTimeWithoutConfirmation = 1 << 1,
        SetTimerConfirmationDefault = 2 << 1,
    };

private:
    /// timer widget showing time since beginning of the presentation
    QLineEdit *passed;
    /// widget showing total time of presentation (as planned)
    QLineEdit *total;
    /// label "/" shown between passed and total
    QLabel *label;
    /// timer which causes updates of passed every second
    QTimer *timer;
    /// map relative times (in ms) to colors to indicate progress
    /// relative to a plan
    QMap<qint32, QRgb> colormap = default_timer_colormap;
    /// target time of current page (planned). This is used to adjust
    /// the color of passed.
    quint32 page_target_time = UINT32_MAX;

    unsigned char _flags = 0;

    void updateTimeout() noexcept;

public:
    explicit TimerWidget(QWidget *parent = NULL);
    ~TimerWidget();

    bool hasHeightForWidth() const noexcept override
    {return true;}

    QSize sizeHint() const noexcept override
    {return {150, 20};}

    /// Get total time passed in ms.
    quint32 timePassed() const noexcept;

    /// Overwrite colormap.
    void setColorMap(QMap<qint32, QRgb> &cmap) noexcept
    {colormap = cmap;}

    /// Map time to color using colormap.
    QColor time2color(const qint32 time) const noexcept;

    unsigned char &flags() noexcept
    {return _flags;}

protected:
    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

    /// Double click event: show option to save current time as end time of current slide.
    void mouseDoubleClickEvent(QMouseEvent*) override;

private slots:
    /// "passed" time was edited. Read it and update accordingly.
    void changePassed();
    /// total time was edited. Read it and update accordingly.
    void changeTotal();

public slots:
    /// Update passed time text and color. Check for timeout.
    void updateText() noexcept;
    /// Update total time widget from preferences(), then call updateText().
    void updateFullText() noexcept;
    /// Handle action: only timer start/pause/toggle/reset actions are handled.
    void handleAction(const Action action) noexcept;
    /// (re)start the timer.
    void startTimer() noexcept;
    /// pause the timer.
    void stopTimer() noexcept;
    /// update per-page time: adjust to given page.
    void updatePage(const int page) noexcept;
    /// Set total time: show in widget and write to preferences().
    void setTotalTime(const QTime time) noexcept;

signals:
    /// Notify about timeout.
    void sendTimeout(const bool timeout);
    /// Update timer status.
    void updateStatus(const Action action, const int status);
    /// Set timer for page (sent to PdfMaster).
    void setTimeForPage(const int page, const quint32 time);
    /// Ask to adjust time as per-page time for given page (sent to PdfMaster).
    void getTimeForPage(const int page, quint32 &time) const;
};

#endif // TIMERWIDGET_H
