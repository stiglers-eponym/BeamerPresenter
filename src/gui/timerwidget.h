#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>
#include "src/enumerates.h"

class QResizeEvent;
class QLineEdit;
class QLabel;
class QTimer;

/// Map time (in ms) left for a slide to color.
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
    enum Flags {
        Timeout = 1 << 0,
        SetTimeWithoutConfirmation = 1 << 1,
        SetTimerConfirmationDefault = 2 << 1,
    };

private:
    QLineEdit *passed;
    QLineEdit *total;
    QLabel *label;
    QTimer *timer;
    quint32 page_target_time = UINT32_MAX;
    QMap<qint32, QRgb> colormap = default_timer_colormap;

    char _flags = 0;

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
    void setColorMap(QMap<qint32, QRgb> &map) noexcept
    {colormap = map;}

    /// Map time to color using colormap.
    QColor time_colormap(const qint32 time) const noexcept;

    char &flags() noexcept
    {return _flags;}

protected:
    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

    /// Double click event: show option to save current time as end time of current slide.
    void mouseDoubleClickEvent(QMouseEvent*) override;

private slots:
    void changePassed();
    void changeTotal();

public slots:
    void updateText() noexcept;
    void updateFullText() noexcept;
    void handleAction(const Action action) noexcept;
    void startTimer() noexcept;
    void stopTimer() noexcept;
    void updatePage(const int page) noexcept;

signals:
    void sendTimeout(const bool timeout);
    void setTimeForPage(const int page, const quint32 time);
    void getTimeForPage(const int page, quint32 &time) const;
};

#endif // TIMERWIDGET_H
