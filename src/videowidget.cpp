#include "videowidget.h"

VideoWidget::VideoWidget(Poppler::MovieObject * movie, QWidget * parent) : QVideoWidget(parent)
{
    setMouseTracking(true);
    player = new QMediaPlayer(this);
    player->setVideoOutput(this);
    QPalette * palette = new QPalette();
    QImage image = movie->posterImage();
    if (!image.isNull())
        palette->setBrush( QPalette::Window, QBrush(image));
    setPalette(*palette);
    delete palette;
    setAutoFillBackground(true);

    QUrl url = QUrl(movie->url());
    if (!url.isValid())
        QUrl url = QUrl::fromLocalFile(movie->url());
    if (url.isRelative())
        url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
    player->setMedia( url );
    // TODO: Check if poster image is shown correctly, otherwise show it with
    // if (movie->showPosterImage())
    switch (movie->playMode()) {
        case Poppler::MovieObject::PlayOpen:
            // TODO: PlayOpen should leave controls open (but they are not implemented yet).
        case Poppler::MovieObject::PlayOnce:
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::showPosterImage);
        break;
        case Poppler::MovieObject::PlayPalindrome:
            std::cout << "WARNING: play mode=palindrome does not work as it should." << std::endl;
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::bouncePalindromeVideo);
        break;
        case Poppler::MovieObject::PlayRepeat:
            connect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::restartVideo);
        break;
    }
    if (movie->showControls()) {
        // TODO: video control bar
    }
    // Scale the video such that it fits the widget size.
    // I like these results, but I don't know whether this is what other people would expect.
    // Please write me a comment if you would prefer an other way of handling the aspect ratio.
    setAspectRatioMode(Qt::IgnoreAspectRatio);
}

VideoWidget::~VideoWidget()
{
    player->stop();
    disconnect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::showPosterImage);
    disconnect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::bouncePalindromeVideo);
    disconnect(player, &QMediaPlayer::stateChanged, this, &VideoWidget::restartVideo);
    delete player;
}

void VideoWidget::play()
{
    show();
    player->play();
}

void VideoWidget::showPosterImage(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
        update();
}

void VideoWidget::bouncePalindromeVideo(QMediaPlayer::State state)
{
    // TODO: The result of this function is not what it should be
    if (state == QMediaPlayer::StoppedState) {
        player->pause();
        player->setPlaybackRate(-player->playbackRate());
        if (player->playbackRate() > 0)
            player->setPosition(0);
        else
            player->setPosition(player->duration());
        player->play();
    }
}

void VideoWidget::restartVideo(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState) {
        if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
            player->setPosition(0);
        player->play();
    }
}

void VideoWidget::mouseReleaseEvent(QMouseEvent * event)
{
    if ( event->button() == Qt::LeftButton ) {
        if ( player->state() == QMediaPlayer::PlayingState )
            player->pause();
        else
            player->play();
    }
    event->accept();
}

void VideoWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (cursor() != Qt::PointingHandCursor)
        setCursor(Qt::PointingHandCursor);
    event->accept();
}
