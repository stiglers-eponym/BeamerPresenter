// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <algorithm>
#include <QTimer>
#include "src/rendering/mediaplayer.h"
#include "src/log.h"

MediaPlayer::MediaPlayer(QObject *parent) :
    QMediaPlayer(parent),
    timer(new QTimer(this))
{
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MediaPlayer::checkPosition);
}

MediaPlayer::~MediaPlayer() noexcept
{
    delete timer;
}

void MediaPlayer::checkPosition()
{
    debug_msg(DebugMedia, "check position" << seekpos << position() << mediaStatus() << playbackState());
    if (seekpos >= 0 && isSeekable())
    {
        setPosition(std::min(seekpos, duration()-1));
        debug_msg(DebugMedia, "done:" << position() << duration() << mediaStatus() << playbackState());
        seekpos = -1;
    }
}

void MediaPlayer::setPositionSoft(int position) noexcept
{
    seekpos = qint64(position);
    timer->start(50);
}
