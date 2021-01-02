#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H

#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include "src/preferences.h"

class TimerWidget : public QWidget
{
    Q_OBJECT
    QLineEdit *passed;
    QLineEdit *total;
    QTimer *timer;
    bool timeout = false;

    void updateTimeout() noexcept;

public:
    explicit TimerWidget(QWidget *parent = nullptr);
    ~TimerWidget();

    bool hasHeightForWidth() const noexcept override
    {return true;}

    QSize sizeHint() const noexcept override
    {return {150, 20};}

protected:
    /// Resize event: adjust font size.
    void resizeEvent(QResizeEvent *event) noexcept override;

private slots:
    void changePassed();
    void changeTotal();

public slots:
    void updateText() noexcept;
    void updateFullText() noexcept;
    void handleAction(const Action action) noexcept;
    void startTimer() noexcept;
    void stopTimer() noexcept;

signals:
    void sendTimeout(const bool timeout);
};

#endif // TIMERWIDGET_H
