#ifndef MEDIASLIDER_H
#define MEDIASLIDER_H

#include <QSlider>
#include "src/config.h"

/**
 * @brief extension of QSlider
 *
 * horizontal slider which accepts qint64 values for maximum and value.
 *
 * @see MediaPlayer
 */
class MediaSlider : public QSlider
{
    Q_OBJECT

public:
    /// Trivial constructor.
    explicit MediaSlider(QWidget *parent = NULL) : QSlider(Qt::Horizontal, parent) {}
    /// Trivial destructor.
    ~MediaSlider() noexcept {}

public slots:
    /// Set maximum (time in ms).
    void setMaximumInt64(qint64 maximum)
    {setMaximum(int(maximum));}

    /// Set position (time in ms).
    void setValueInt64(qint64 value)
    {setValue(int(value));}
};

#endif // MEDIASLIDER_H
