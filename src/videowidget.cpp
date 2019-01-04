#include "videowidget.h"

VideoWidget::VideoWidget(Poppler::MovieAnnotation const * annotation, QWidget * parent) : QVideoWidget(parent)
{
    this->annotation = annotation;
    setMouseTracking(true);
    player = new QMediaPlayer(this);
    player->setVideoOutput(this);
    Poppler::MovieObject * movie = annotation->movie();
    if (movie->showPosterImage()) {
        QPalette * palette = new QPalette();
        posterImage = movie->posterImage();
        if (!posterImage.isNull())
            palette->setBrush( QPalette::Window, QBrush(posterImage));
        setPalette(*palette);
        delete palette;
        setAutoFillBackground(true);
    }

    QUrl url = QUrl(movie->url());
    if (!url.isValid())
        QUrl url = QUrl::fromLocalFile(movie->url());
    if (url.isRelative())
        url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
    player->setMedia( url );
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
    delete annotation;
    delete player;
}

Poppler::MovieAnnotation const * VideoWidget::getAnnotation()
{
    return annotation;
}

void VideoWidget::play()
{
    if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
        player->bind(this);
    player->play();
}

void VideoWidget::pause()
{
    if (player->state() == QMediaPlayer::PlayingState)
        player->pause();
}

QMediaPlayer::State VideoWidget::state() const
{
    return player->state();
}

void VideoWidget::showPosterImage(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState && player->mediaStatus() == QMediaPlayer::EndOfMedia) {
        // Unbinding and binding this to the player is probably an ugly way of solving this.
        // TODO: find a better way of writing this.
        player->unbind(this);
        show();
    }
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
        else {
            if (player->mediaStatus() == QMediaPlayer::EndOfMedia)
                player->bind(this);
            player->play();
        }
    }
    event->accept();
}

void VideoWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (cursor() != Qt::PointingHandCursor)
        setCursor(Qt::PointingHandCursor);
    event->accept();
}
