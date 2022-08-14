// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include "src/rendering/mediaplayer.h"
#include "src/preferences.h"

MediaPlayer::MediaPlayer(QObject *parent) :
    QMediaPlayer(parent),
    timer(new QTimer(this))
{
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MediaPlayer::checkPosition);
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
