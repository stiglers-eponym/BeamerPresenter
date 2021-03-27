#include "src/gui/timerwidget.h"
#include "src/preferences.h"
#include <QHBoxLayout>
#include <QMessageBox>
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
    connect(passed, &QLineEdit::returnPressed, this, &TimerWidget::changePassed);
    connect(total, &QLineEdit::editingFinished, this, &TimerWidget::changeTotal);
    passed->setAlignment(Qt::AlignCenter);
    total->setAlignment(Qt::AlignCenter);
    passed->setToolTip("time since beginning of the presentation");
    total->setToolTip("total duration of the presentation");
    setToolTip("double-click here to set time for this slide for your orientation while presenting");
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
    if (_flags & Timeout)
        palette.setColor(QPalette::Base, Qt::red);
    else
        palette.setColor(QPalette::Base, Qt::white);
    setPalette(palette);
    emit sendTimeout(_flags & Timeout);
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
    const QTime time = QTime::fromMSecsSinceStartOfDay(preferences()->msecs_total);
    total->setText(time.toString(time.hour() ? "h:mm:ss" : "m:ss"));
    updateText();
}

void TimerWidget::updateText() noexcept
{
    const quint32 msecs_passed = timePassed();
    if (!passed->hasFocus())
        passed->setText(
                    QTime::fromMSecsSinceStartOfDay(msecs_passed).toString(
                            msecs_passed < 3600000 ? "m:ss" : "h:mm:ss"
                    )
                );
    if (!(_flags & Timeout) && msecs_passed >= preferences()->msecs_total && preferences()->msecs_total > 0)
    {
        _flags |= Timeout;
        updateTimeout();
    }
    if (page_target_time == UINT32_MAX)
    {
        QPalette palette;
        palette.setColor(QPalette::Base, colormap.last());
        passed->setPalette(palette);
    }
    else
    {
        const qint32 diff = page_target_time - msecs_passed;
        QPalette palette;
        palette.setColor(QPalette::Base, time_colormap(diff));
        passed->setPalette(palette);
    }
}

void TimerWidget::changePassed()
{
    const quint32 new_msecs_passed = QTime::fromString(passed->text(), total->text().count(":") == 2 ? "h:mm:ss" : "m:ss").msecsSinceStartOfDay();
    debug_msg(DebugWidgets) << "new msecs passed:" << new_msecs_passed;
    if (preferences()->msecs_passed == UINT_LEAST32_MAX)
        writable_preferences()->target_time = QDateTime::currentDateTimeUtc().addMSecs(preferences()->msecs_total - new_msecs_passed);
    else
        writable_preferences()->msecs_passed = new_msecs_passed;
    if (preferences()->msecs_total > 0 && new_msecs_passed >= preferences()->msecs_total)
        _flags |= Timeout;
    else
        _flags &= ~Timeout;
    updateTimeout();
    updateFullText();
}

void TimerWidget::changeTotal()
{
    setTotalTime(
                QTime::fromString(
                    total->text(),
                    total->text().count(":") == 2 ? "h:mm:ss" : "m:ss"
                )
            );
}

void TimerWidget::setTotalTime(const QTime time) noexcept
{
    debug_msg(DebugWidgets) << time << time.isValid();
    if (time.isValid())
    {
        const quint32 msecs_total = time.msecsSinceStartOfDay();
        if (preferences()->msecs_passed == UINT_LEAST32_MAX)
        {
            writable_preferences()->target_time = preferences()->target_time.addMSecs(msecs_total - preferences()->msecs_total);
            if (msecs_total > 0 && QDateTime::currentDateTimeUtc() >= preferences()->target_time)
                _flags |= Timeout;
            else
                _flags &= ~Timeout;
        }
        else
        {
            if (msecs_total > 0 && msecs_total <= preferences()->msecs_passed)
                _flags |= Timeout;
            else
                _flags &= ~Timeout;
        }
        updateTimeout();
        writable_preferences()->msecs_total = msecs_total;
        updateFullText();
    }
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
        _flags &= ~Timeout;
        updateTimeout();
        updateFullText();
        break;
    default:
        break;
    }
}

quint32 TimerWidget::timePassed() const noexcept
{
    return preferences()->msecs_passed == UINT_LEAST32_MAX
        ? preferences()->msecs_total - QDateTime::currentDateTimeUtc().msecsTo(preferences()->target_time)
        : preferences()->msecs_passed;
}

void TimerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    if (passed->hasFocus())
        changePassed();
    if (
            (_flags & SetTimeWithoutConfirmation)
            || QMessageBox::question(
                    this,
                    "Timer",
                    "Save current time as target end time for current slide?",
                    QMessageBox::Yes | QMessageBox::No,
                    _flags & SetTimerConfirmationDefault ? QMessageBox::Yes : QMessageBox::No
                ) == QMessageBox::Yes
        )
    {
        page_target_time = timePassed();
        emit setTimeForPage(preferences()->page, page_target_time);
        updateFullText();
    }
}

QColor TimerWidget::time_colormap(const qint32 time) const noexcept
{
    const auto next_it = colormap.lowerBound(time);
    if (next_it == colormap.cend())
        return colormap.last();
    if (next_it == colormap.cbegin())
        return colormap.first();

    const int diff = next_it.key() - time;
    const int total = next_it.key() - (next_it-1).key();
    return QColor(((total-diff)*qRed(*next_it) + diff*qRed(*(next_it-1)))/total, ((total-diff)*qGreen(*next_it) + diff*qGreen(*(next_it-1)))/total, ((total-diff)*qBlue(*next_it) + diff*qBlue(*(next_it-1)))/total);
}

void TimerWidget::updatePage(const int page) noexcept
{
    emit getTimeForPage(page, page_target_time);
    updateText();
}
