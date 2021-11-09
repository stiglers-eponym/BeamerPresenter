#include "src/rendering/mediaplayer.h"

MediaPlayer::MediaPlayer(QObject *parent) :
    QMediaPlayer(parent),
    timer(new QTimer(this))
{
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MediaPlayer::checkPosition);
}

void MediaPlayer::repeatIfFinished(MediaPlayer::MediaStatus status) noexcept
{
    if (status == QMediaPlayer::EndOfMedia)
        play();
}

void MediaPlayer::checkPosition()
{
    debug_msg(DebugMedia, "check position" << seekpos << position());
    if (seekpos >= 0)
    {
        setPosition(seekpos);
        seekpos = -1;
    }
}
