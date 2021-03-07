#include "src/gui/timerwidget.h"
#include "src/preferences.h"
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>

TimerWidget::TimerWidget(QWidget *parent) :
     QWidget(parent),
     passed(new QLineEdit(this)),
     total(new QLineEdit(this)),
     label(new QLabel(" / ", this)),
     timer(new QTimer(this))
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(passed);
    layout->addWidget(label);
    layout->addWidget(total);
    setLayout(layout);
    connect(timer, &QTimer::timeout, this, &TimerWidget::updateText);
    timer->setTimerType(Qt::CoarseTimer);
    connect(passed, &QLineEdit::editingFinished, this, &TimerWidget::changePassed);
    connect(total, &QLineEdit::editingFinished, this, &TimerWidget::changeTotal);
    updateFullText();
}

TimerWidget::~TimerWidget()
{
    delete timer;
    timer = NULL;
    delete passed;
    passed = NULL;
    delete total;
    total = NULL;
}

void TimerWidget::updateTimeout() noexcept
{
    QPalette palette;
    if (timeout)
        palette.setColor(QPalette::Base, Qt::red);
    else
        palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    emit sendTimeout(timeout);
}

void TimerWidget::resizeEvent(QResizeEvent *event) noexcept
{
    QFont font({"", std::min(event->size().height()/2, event->size().width()/12), 2});
    passed->setFont(font);
    total->setFont(font);
    font.setPointSizeF(0.75*font.pointSizeF());
    label->setFont(font);
}

void TimerWidget::updateFullText() noexcept
{
    total->setText(
                QTime::fromMSecsSinceStartOfDay(
                    preferences()->msecs_total + 500
                ).toString(preferences()->msecs_total < 3600000 ? "m:ss" : "h:mm:ss")
            );
    updateText();
}

void TimerWidget::updateText() noexcept
{
    if (preferences()->msecs_passed == UINT_LEAST32_MAX)
    {
        const quint32 msecs_passed = preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time);
        if (msecs_passed < 3600000)
        {
            passed->setText(QTime::fromMSecsSinceStartOfDay(msecs_passed + 500).toString("m:ss"));
            if (!timeout && msecs_passed >= preferences()->msecs_total)
            {
                timeout = true;
                updateTimeout();
            }
        }
        else
            passed->setText(QTime::fromMSecsSinceStartOfDay(msecs_passed + 500).toString("h:mm:ss"));
    }
    else
        passed->setText(
                QTime::fromMSecsSinceStartOfDay(
                    preferences()->msecs_passed + 500
                ).toString(preferences()->msecs_passed < 3600000 ? "m:ss" : "h:mm:ss")
            );
}

void TimerWidget::changePassed()
{
    const qint64 new_msecs_passed = QTime::fromString(passed->text(), total->text().count(":") == 2 ? "h:mm:ss" : "m:ss").msecsSinceStartOfDay();
    debug_msg(DebugWidgets) << "new msecs passed:" << new_msecs_passed;
    if (preferences()->msecs_passed == UINT_LEAST32_MAX)
    {
        writable_preferences()->target_time = QDateTime::currentDateTimeUtc().addMSecs(preferences()->msecs_total - new_msecs_passed);
        timeout = QDateTime::currentDateTimeUtc() >= preferences()->target_time;
    }
    else
    {
        writable_preferences()->msecs_passed = new_msecs_passed;
        timeout = preferences()->msecs_total <= new_msecs_passed;
    }
    updateTimeout();
    updateFullText();
}

void TimerWidget::changeTotal()
{
    const qint64 msecs_total = QTime::fromString(total->text(), total->text().count(":") == 2 ? "h:mm:ss" : "m:ss").msecsSinceStartOfDay();
    debug_msg(DebugWidgets) << "msecs total:" << msecs_total;
    if (preferences()->msecs_passed == UINT_LEAST32_MAX)
    {
        writable_preferences()->target_time = preferences()->target_time.addMSecs(msecs_total - preferences()->msecs_total);
        timeout = QDateTime::currentDateTimeUtc() >= preferences()->target_time;
    }
    else
        timeout = msecs_total <= preferences()->msecs_passed;
    updateTimeout();
    writable_preferences()->msecs_total = msecs_total;
    updateFullText();
}

void TimerWidget::startTimer() noexcept
{
    if (preferences()->msecs_passed != UINT_LEAST32_MAX)
    {
        writable_preferences()->target_time = QDateTime::currentDateTimeUtc().addMSecs(preferences()->msecs_total - preferences()->msecs_passed);
        writable_preferences()->msecs_passed = UINT_LEAST32_MAX;
    }
    timer->start(1000);
}

void TimerWidget::stopTimer() noexcept
{
    timer->stop();
    if (preferences()->msecs_passed == UINT_LEAST32_MAX)
    {
        writable_preferences()->msecs_passed = preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time);
        updateFullText();
    }
}

void TimerWidget::handleAction(const Action action) noexcept
{
    switch (action)
    {
    case StartTimer:
        startTimer();
        break;
    case StopTimer:
        stopTimer();
        break;
    case StartStopTimer:
        if (preferences()->msecs_passed == UINT_LEAST32_MAX)
            stopTimer();
        else
            startTimer();
        break;
    case ResetTimePassed:
        if (preferences()->msecs_passed == UINT_LEAST32_MAX)
            writable_preferences()->target_time = QDateTime::currentDateTimeUtc().addMSecs(preferences()->msecs_total);
        else
            writable_preferences()->msecs_passed = 0;
        timeout = false;
        updateTimeout();
        updateFullText();
        break;
    default:
        break;
    }
}
