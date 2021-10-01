#ifndef MEDIASLIDER_H
#define MEDIASLIDER_H

#include <QSlider>

/**
 * @brief MediaSlider class, extension of QSlider
 *
 * horizontal slider which accepts qint64 values for maximum and value.
 */
class MediaSlider : public QSlider
{
    Q_OBJECT

public:
    explicit MediaSlider(QWidget *parent = NULL) : QSlider(Qt::Horizontal, parent) {}
    ~MediaSlider() noexcept {}

public slots:
    void setMaximumInt64(qint64 maximum)
    {setMaximum(int(maximum));}

    void setValueInt64(qint64 value)
    {setValue(int(value));}
};

#endif // MEDIASLIDER_H
