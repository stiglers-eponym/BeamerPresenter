// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

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
    debug_msg(DebugMedia, "check position" << seekpos << position());
    if (seekpos >= 0)
    {
        setPosition(seekpos);
        seekpos = -1;
    }
}

void MediaPlayer::setPositionSoft(int position) noexcept
{
    seekpos = qint64(position);
    timer->start(50);
}
