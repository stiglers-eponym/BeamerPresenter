#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QWidget>
#include "src/enumerates.h"

class QResizeEvent;
class QLineEdit;
class QLabel;
class QTimer;

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

    QLineEdit *passed;
    QLineEdit *total;
    QLabel *label;
    QTimer *timer;
    bool timeout = false;
    quint32 page_target_time = UINT32_MAX;

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

QColor time_colormap(const qint32 time) noexcept;

/// Map time (in ms) left for a slide to color.
static const QMap<qint32, QRgb> colormap
{
    {-60000, qRgb(255,   0,   0)},
    {-30000, qRgb(255, 255,   0)},
    {     0, qRgb(  0, 255,   0)},
    { 30000, qRgb(  0, 255, 255)},
    { 60000, qRgb(255, 255, 255)},
};

#endif // TIMERWIDGET_H
