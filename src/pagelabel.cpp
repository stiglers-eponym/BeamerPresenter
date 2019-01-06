/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include "pagelabel.h"

PageLabel::PageLabel(Poppler::Page* page, QWidget* parent) : QLabel(parent)
{
    renderPage(page);
}

PageLabel::PageLabel(QWidget* parent) : QLabel(parent)
{
    page = nullptr;
    links = QList<Poppler::Link*>();
    linkPositions = QList<QRect*>();
    videoWidgets = QList<VideoWidget*>();
    videoPositions = QList<QRect*>();
    soundPositions = QList<QRect*>();
    soundPlayers = QList<QMediaPlayer*>();
}

PageLabel::~PageLabel()
{
    delete timer;
    clearLists();
    page = nullptr;
}

void PageLabel::setAutostartDelay(double const delay)
{
    autostartDelay = delay;
}

int PageLabel::pageNumber()
{
    return page->index();
}

void PageLabel::setAnimationDelay(int const delay_ms)
{
    minimumAnimationDelay = delay_ms;
}

void PageLabel::clearLists()
{
    if (sliders.size() != 0) {
        if (sliders.size() == videoWidgets.size() + soundPlayers.size()) {
            for (int i=0; i<videoWidgets.size(); i++) {
                MediaSlider * slider = sliders.at(i);
                VideoWidget * video = videoWidgets.at(i);
                disconnect(video->getPlayer(), &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
                disconnect(slider, &MediaSlider::sliderMoved, video, &VideoWidget::setPosition);
                disconnect(video->getPlayer(), &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
            }
            for (int i=0; i<soundPlayers.size(); i++) {
                MediaSlider * slider = sliders.at(i + videoWidgets.size());
                QMediaPlayer * player = soundPlayers.at(i);
                disconnect(player, &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
                disconnect(slider, &MediaSlider::sliderMoved, player, &QMediaPlayer::setPosition);
                disconnect(player, &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
            }
        }
        else {
            std::cout << "Something unexpected happened while trying to delete up sliders." << std::endl;
        }
    }
    qDeleteAll(sliders);
    sliders.clear();
    qDeleteAll(links);
    links.clear();
    qDeleteAll(linkPositions);
    linkPositions.clear();
    qDeleteAll(videoPositions);
    videoPositions.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    qDeleteAll(soundPositions);
    soundPositions.clear();
    qDeleteAll(soundPlayers);
    soundPlayers.clear();
}

void PageLabel::renderPage(Poppler::Page * page)
{
    clearLists();

    this->page = page;
    QSize pageSize = page->pageSize();
    int shift_x=0, shift_y=0;
    if ( width() * pageSize.height() > height() * pageSize.width() ) {
        // the width of the label is larger than required
        resolution = double( height() ) / pageSize.height();
        shift_x = int( width()/2 - resolution/2 * pageSize.width() );
    }
    else {
        // the height of the label is larger than required
        resolution = double( width() ) / pageSize.width();
        shift_y = int( height()/2 - resolution/2 * pageSize.height() );
    }
    double scale_x=resolution*pageSize.width(), scale_y=resolution*pageSize.height();
    if (page->index() == cachedIndex)
        setPixmap( cachedPixmap );
    else
        setPixmap( QPixmap::fromImage( page->renderToImage( 72*resolution, 72*resolution ) ) );

    // Collect link areas in pixels
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        QRect * absolute = new QRect(
                    shift_x+int(relative.x()*scale_x),
                    shift_y+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                );
        linkPositions.append( absolute );
    }

    duration = page->duration();
    if ( duration > 0.01)
        QTimer::singleShot(int(1000*duration), this, &PageLabel::timeoutSignal);
    else if ( duration > -0.01) {
        update();
        QTimer::singleShot(int(minimumAnimationDelay), this, &PageLabel::timeoutSignal);
    }
    Poppler::PageTransition* transition = page->transition();
    if (transition->type() != Poppler::PageTransition::Replace)
        std::cout << "Unsupported page transition of type " << transition->type() << std::endl;

    // Show videos. This part is work in progress.
    // TODO: This is probably inefficient.
    if (showMultimedia) {
        // Video
        QSet<Poppler::Annotation::SubType> videoType = QSet<Poppler::Annotation::SubType>();
        videoType.insert(Poppler::Annotation::AMovie);
        QList<Poppler::Annotation*> videos = page->annotations(videoType);
        for (int i=0; i<videos.size(); i++) {
            Poppler::MovieAnnotation * annotation = (Poppler::MovieAnnotation*) videos.at(i);
            QRectF relative = annotation->boundary();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            videoPositions.append(absolute);

            VideoWidget * video = new VideoWidget(annotation, this);
            videoWidgets.append(video);
        }
        videos.clear();

        // Audio (Untested!)
        QSet<Poppler::Annotation::SubType> soundType = QSet<Poppler::Annotation::SubType>();
        soundType.insert(Poppler::Annotation::ASound);
        QList<Poppler::Annotation*> sounds = page->annotations(soundType);
        for (int i=0; i<sounds.size(); i++) {
            std::cout << "WARNING: Support for sound is untested!" << std::endl;
            Poppler::SoundAnnotation* annotation = (Poppler::SoundAnnotation*) sounds.at(i);
            QRectF relative = annotation->boundary();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            soundPositions.append(absolute);

            Poppler::SoundObject * sound = annotation->sound();
            QMediaPlayer * player = new QMediaPlayer(this);
            QUrl url = QUrl(sound->url());
            if (!url.isValid())
                QUrl url = QUrl::fromLocalFile(sound->url());
            if (url.isRelative())
                url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
            player->setMedia( url );
            soundPlayers.append(player);
        }
        qDeleteAll(sounds);
        sounds.clear();

        // Autostart
        if (autostartDelay > 0.1) {
            // autostart with delay
            delete timer;
            timer = new QTimer();
            timer->setSingleShot(true);
            connect(timer, &QTimer::timeout, this, &PageLabel::startAllMultimedia );
            timer->start(int(autostartDelay*1000));
        }
        else if (autostartDelay > -0.1) {
            // autostart without delay
            startAllMultimedia();
        }

        // Add slider
        emit requestMultimediaSliders(videoWidgets.size() + soundPlayers.size());
    }
}

void PageLabel::updateCache(QPixmap * pixmap, int const index)
{
    cachedIndex = index;
    cachedPixmap = *pixmap;
}

void PageLabel::updateCache(Poppler::Page * nextPage)
{
    if ((page->duration() < -0.01  || page->duration() > 0.1) && (nextPage->index() != cachedIndex)) {
        cachedIndex = nextPage->index();
        cachedPixmap = QPixmap::fromImage( nextPage->renderToImage(72*resolution, 72*resolution) );
    }
}

QPixmap * PageLabel::getCache()
{
    return &cachedPixmap;
}

int PageLabel::getCacheIndex() const
{
    return cachedIndex;
}

void PageLabel::setMultimediaSliders(QList<MediaSlider *> sliderList)
{
    if (sliders.size() != 0) {
        std::cout << "Something unexpected happened: There is a problem with the media sliders." << std::endl;
        return;
    }
    sliders = sliderList;
    for (int i=0; i<videoWidgets.size(); i++) {
        MediaSlider * slider = sliders.at(i);
        VideoWidget * video = videoWidgets.at(i);
        connect(video->getPlayer(), &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
        int const duration = int(video->getDuration()/100);
        if (duration > 0)
            slider->setMaximum(duration);
        connect(slider, &MediaSlider::sliderMoved, video, &VideoWidget::setPosition);
        connect(video->getPlayer(), &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
    }
    for (int i=0; i<soundPlayers.size(); i++) {
        MediaSlider * slider = sliders.at(i + videoWidgets.size());
        QMediaPlayer * player = soundPlayers.at(i);
        slider->setRange(0, int(player->duration()));
        connect(player, &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
        int const duration = int(player->duration()/100);
        if (duration > 0)
            slider->setMaximum(duration);
        connect(slider, &MediaSlider::sliderMoved, player, &QMediaPlayer::setPosition);
        connect(player, &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
    }
    show();
}

void PageLabel::startAllMultimedia()
{
    for (int i=0; i<videoWidgets.size(); i++) {
        videoWidgets.at(i)->setGeometry(*videoPositions.at(i));
        videoWidgets.at(i)->show();
        videoWidgets.at(i)->play();
    }
    Q_FOREACH(QMediaPlayer * sound, soundPlayers)
        sound->play();
}

void PageLabel::pauseAllMultimedia()
{
    Q_FOREACH(VideoWidget * video, videoWidgets)
        video->pause();
    Q_FOREACH(QMediaPlayer * sound, soundPlayers)
        sound->pause();
}

bool PageLabel::hasActiveMultimediaContent() const
{
    Q_FOREACH(VideoWidget * video, videoWidgets) {
        if (video->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer * sound, soundPlayers) {
        if (sound->state() == QMediaPlayer::PlayingState)
            return true;
    }
    return false;
}

void PageLabel::setPresentationStatus(bool const isPresentation)
{
    this->isPresentation = isPresentation;
}

void PageLabel::setShowMultimedia(const bool showMultimedia)
{
    this->showMultimedia = showMultimedia;
}

double PageLabel::getDuration() const
{
    return duration;
}

void PageLabel::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            if ( linkPositions.at(i)->contains(event->pos()) ) {
                switch ( links.at(i)->linkType() )
                {
                    case Poppler::Link::Goto:
                        emit sendNewPageNumber( ((Poppler::LinkGoto*) links.at(i))->destination().pageNumber() - 1 );
                    break;
                    case Poppler::Link::Execute:
                        {
                            std::cout << "Unsupported link of type execute:" << std::endl;
                            Poppler::LinkExecute * link = (Poppler::LinkExecute*) links.at(i);
                            if (!link->parameters().isEmpty())
                                std::cout << "Execution parameters: " << link->parameters().toStdString() << std::endl;
                            std::cout << "File to be executed: " << link->fileName().toStdString() << std::endl;
                        }
                    break;
                    case Poppler::Link::Browse:
                        QDesktopServices::openUrl( QUrl( ((Poppler::LinkBrowse*) links.at(i))->url(), QUrl::TolerantMode ) );
                    break;
                    case Poppler::Link::Action:
                        {
                            Poppler::LinkAction* link = (Poppler::LinkAction*) links.at(i);
                            switch (link->actionType())
                            {
                                case Poppler::LinkAction::Quit:
                                case Poppler::LinkAction::Close:
                                    emit sendCloseSignal();
                                    break;
                                case Poppler::LinkAction::Print:
                                    std::cout << "Unsupported link action: print." << std::endl;
                                    break;
                                case Poppler::LinkAction::GoToPage:
                                    emit focusPageNumberEdit();
                                    break;
                                case Poppler::LinkAction::PageNext:
                                    emit sendNewPageNumber( pageNumber() + 1 );
                                    break;
                                case Poppler::LinkAction::PagePrev:
                                    emit sendNewPageNumber( pageNumber() - 1 );
                                    break;
                                case Poppler::LinkAction::PageFirst:
                                    emit sendNewPageNumber(0);
                                    break;
                                case Poppler::LinkAction::PageLast:
                                    emit sendNewPageNumber(-1);
                                    break;
                                case Poppler::LinkAction::Find:
                                    // TODO: implement this
                                    std::cout << "Unsupported link action: find." << std::endl;
                                    break;
                                case Poppler::LinkAction::Presentation:
                                    std::cout << "Unsupported link action: presentation." << std::endl;
                                    std::cout << "This pdf viewer is always in presentation mode." << std::endl;
                                    break;
                                case Poppler::LinkAction::EndPresentation:
                                    std::cout << "Unsupported link action: end presentation." << std::endl;
                                    std::cout << "This pdf viewer is always in presentation mode." << std::endl;
                                    break;
                                case Poppler::LinkAction::HistoryBack:
                                    // TODO: implement this
                                    std::cout << "Unsupported link action: history back." << std::endl;
                                    break;
                                case Poppler::LinkAction::HistoryForward:
                                    // TODO: implement this
                                    std::cout << "Unsupported link action: history forward." << std::endl;
                                    break;
                            };
                        }
                    break;
                    case Poppler::Link::Sound:
                        std::cout << "Unsupported link of type sound" << std::endl;
                    break;
                    case Poppler::Link::Movie:
                        {
                            std::cout << "Unsupported link of type video." << std::endl;
                            Poppler::LinkMovie* link = (Poppler::LinkMovie*) links.at(i);
                            Q_FOREACH(VideoWidget * video, videoWidgets) {
                                if (link->isReferencedAnnotation(video->getAnnotation()))
                                    video->play();
                            }
                        }
                    break;
                    case Poppler::Link::Rendition:
                        std::cout << "Unsupported link of type rendition" << std::endl;
                    break;
                    case Poppler::Link::JavaScript:
                        std::cout << "Unsupported link of type JavaScript" << std::endl;
                    break;
                    case Poppler::Link::OCGState:
                        if ( linkPositions.at(i)->contains(event->pos()) )
                            std::cout << "Unsupported link of type OCGState" << std::endl;
                    break;
                    case Poppler::Link::Hide:
                        std::cout << "Unsupported link of type hide" << std::endl;
                    break;
                    case Poppler::Link::None:
                        std::cout << "Unsupported link of type none" << std::endl;
                    break;
                }
            }
        }
        for (int i=0; i<soundPositions.size(); i++) {
            if (soundPositions.at(i)->contains(event->pos())) {
                if (soundPlayers.at(i)->state() == QMediaPlayer::PlayingState)
                    soundPlayers.at(i)->pause();
                else
                    soundPlayers.at(i)->play();
            }
        }
    }
    event->accept();
}

void PageLabel::togglePointerVisibility()
{
    if ( pointer_visible ) {
        pointer_visible = false;
        setMouseTracking(false);
        setCursor( Qt::BlankCursor );
    }
    else {
        pointer_visible = true;
        setMouseTracking(true);
        setCursor( Qt::ArrowCursor );
    }
}

void PageLabel::mouseMoveEvent(QMouseEvent * event)
{
    if (!pointer_visible)
        return;
    bool is_arrow_pointer = cursor() == Qt::ArrowCursor;
    Q_FOREACH(QRect* link_rect, linkPositions) {
        if (link_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    Q_FOREACH(QRect* sound_rect, soundPositions) {
        if (sound_rect->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    if (!is_arrow_pointer)
        setCursor(Qt::ArrowCursor);
    event->accept();
}
