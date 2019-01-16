/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pagelabel.h"

PageLabel::PageLabel(Poppler::Page* page, QWidget* parent) : QLabel(parent)
{
    renderPage(page);
}

PageLabel::PageLabel(QWidget* parent) : QLabel(parent)
{
    page = nullptr;
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

int PageLabel::pageNumber() const
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
        if (sliders.size() == videoWidgets.size() + soundPlayers.size() + linkSoundPlayers.size()) {
            for (int i=0; i<videoWidgets.size(); i++) {
                MediaSlider * slider = sliders.at(i);
                VideoWidget * video = videoWidgets.at(i);
                disconnect(video->getPlayer(), &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
                disconnect(slider, &MediaSlider::sliderMoved, video, &VideoWidget::setPosition);
                disconnect(video->getPlayer(), &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
            }
            for (int i=0; i<linkSoundPlayers.size(); i++) {
                MediaSlider * slider = sliders.at(i + videoWidgets.size());
                QMediaPlayer * player = linkSoundPlayers.at(i);
                disconnect(player, &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
                disconnect(slider, &MediaSlider::sliderMoved,      player, &QMediaPlayer::setPosition);
                disconnect(player, &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
            }
            for (int i=0; i<soundPlayers.size(); i++) {
                MediaSlider * slider = sliders.at(i + videoWidgets.size() + linkSoundPlayers.size());
                QMediaPlayer * player = soundPlayers.at(i);
                disconnect(player, &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
                disconnect(slider, &MediaSlider::sliderMoved,      player, &QMediaPlayer::setPosition);
                disconnect(player, &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
            }
        }
        else
            qDebug() << "Something unexpected happened while trying to delete up sliders.";
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
    qDeleteAll(linkSoundPositions);
    linkSoundPositions.clear();
    qDeleteAll(linkSoundPlayers);
    linkSoundPlayers.clear();
    qDeleteAll(embeddedWindows);
    embeddedWindows.clear();
}

void PageLabel::renderPage(Poppler::Page* page, bool setDuration)
{
    emit slideChange();
    clearLists();
    if (page == nullptr)
        return;

    this->page = page;
    QSize pageSize = page->pageSize();
    int shift_x=0, shift_y=0;
    int pageHeight=pageSize.height(), pageWidth=pageSize.width();
    if (pagePart != 0)
        pageWidth /= 2;
    if ( width() * pageHeight > height() * pageWidth ) {
        // the width of the label is larger than required
        resolution = double( height() ) / pageHeight;
        shift_x = int( width()/2 - resolution/2 * pageWidth );
    }
    else {
        // the height of the label is larger than required
        resolution = double( width() ) / pageWidth;
        shift_y = int( height()/2 - resolution/2 * pageHeight );
    }
    double scale_x=resolution*pageWidth, scale_y=resolution*pageHeight;
    if (pagePart !=0) {
        scale_x *= 2;
        if (pagePart == -1)
            shift_x -= width();
    }
    if (page->index() == cachedIndex)
        setPixmap( cachedPixmap );
    else {
        if (pagePart == 0)
            setPixmap( QPixmap::fromImage( page->renderToImage( 72*resolution, 72*resolution ) ) );
        else {
            QImage image = page->renderToImage( 72*resolution, 72*resolution );
            if (pagePart == 1)
                setPixmap( QPixmap::fromImage( image.copy(0, 0, image.width()/2, image.height()) ) );
            else
                setPixmap( QPixmap::fromImage( image.copy(image.width()/2, 0, image.width()/2, image.height()) ) );
        }
    }

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

    if (setDuration) {
        duration = page->duration();
        if ( duration > 0.01)
            QTimer::singleShot(int(1000*duration), this, &PageLabel::timeoutSignal);
        else if ( duration > -0.01) {
            update();
            QTimer::singleShot(int(minimumAnimationDelay), this, &PageLabel::timeoutSignal);
        }
    }
    Poppler::PageTransition* transition = page->transition();
    if (transition != nullptr && transition->type() != Poppler::PageTransition::Replace)
        qInfo() << "Unsupported page transition of type " << transition->type();

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

        // Audio as annotations (Untested, I don't know whether this is useful for anything)
        QSet<Poppler::Annotation::SubType> soundType = QSet<Poppler::Annotation::SubType>();
        soundType.insert(Poppler::Annotation::ASound);
        QList<Poppler::Annotation*> sounds = page->annotations(soundType);
        for (int i=0; i<sounds.size(); i++) {
            qWarning() << "WARNING: Support for sound in annotations is untested!";
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

        // Audio links
        Q_FOREACH(Poppler::Link * link, links) {
            if (link->linkType() != Poppler::Link::Sound)
                continue;
            QRectF relative = link->linkArea();
            QRect * absolute = new QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    );
            linkSoundPositions.append(absolute);

            Poppler::SoundObject * sound = ((Poppler::LinkSound*) link)->sound();
            QMediaPlayer * player = new QMediaPlayer(this);
            QUrl url = QUrl(sound->url());
            if (!url.isValid())
                QUrl url = QUrl::fromLocalFile(sound->url());
            if (url.isRelative())
                url = QUrl::fromLocalFile( QDir(".").absoluteFilePath( url.path()) );
            player->setMedia( url );
            linkSoundPlayers.append(player);
        }

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
        emit requestMultimediaSliders(videoWidgets.size() + linkSoundPlayers.size() + soundPlayers.size());
    }
}

void PageLabel::updateCache(QPixmap* pixmap, int const index)
{
    cachedIndex = index;
    cachedPixmap = *pixmap;
}

void PageLabel::updateCache(Poppler::Page* nextPage)
{
    if (page == nullptr) {
        page = nextPage;
        cachedIndex = nextPage->index();
        if (pagePart == 0)
            cachedPixmap = QPixmap::fromImage( nextPage->renderToImage( 72*resolution, 72*resolution ) );
        else {
            QImage image = nextPage->renderToImage( 72*resolution, 72*resolution );
            if (pagePart == 1)
                cachedPixmap = QPixmap::fromImage( image.copy(0, 0, image.width()/2, image.height()) );
            else
                cachedPixmap = QPixmap::fromImage( image.copy(image.width()/2, 0, image.width()/2, image.height()) );
        }
        return;
    }
    double const nextDuration = nextPage->duration();
    if (nextPage->index() != cachedIndex && ( nextDuration < -0.01  || nextDuration > 0.1) ) {
        cachedIndex = nextPage->index();
        if (pagePart == 0)
            cachedPixmap = QPixmap::fromImage( nextPage->renderToImage( 72*resolution, 72*resolution ) );
        else {
            QImage image = nextPage->renderToImage( 72*resolution, 72*resolution );
            if (pagePart == 1)
                cachedPixmap = QPixmap::fromImage( image.copy(0, 0, image.width()/2, image.height()) );
            else
                cachedPixmap = QPixmap::fromImage( image.copy(image.width()/2, 0, image.width()/2, image.height()) );
        }
    }
}

QPixmap* PageLabel::getCache()
{
    return &cachedPixmap;
}

int PageLabel::getCacheIndex() const
{
    return cachedIndex;
}

void PageLabel::setMultimediaSliders(QList<MediaSlider*> sliderList)
{
    if (sliders.size() != 0) {
        qWarning() << "Something unexpected happened: There is a problem with the media sliders.";
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
    for (int i=0; i<linkSoundPlayers.size(); i++) {
        MediaSlider * slider = sliders.at(i + videoWidgets.size());
        QMediaPlayer * player = linkSoundPlayers.at(i);
        slider->setRange(0, int(player->duration()));
        connect(player, &QMediaPlayer::durationChanged, slider, &MediaSlider::setMaximum);
        int const duration = int(player->duration()/100);
        if (duration > 0)
            slider->setMaximum(duration);
        connect(slider, &MediaSlider::sliderMoved, player, &QMediaPlayer::setPosition);
        connect(player, &QMediaPlayer::positionChanged, slider, &MediaSlider::setValue);
    }
    for (int i=0; i<soundPlayers.size(); i++) {
        MediaSlider * slider = sliders.at(i + videoWidgets.size() + linkSoundPlayers.size());
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
    Q_FOREACH(QMediaPlayer * sound, linkSoundPlayers)
        sound->play();
}

void PageLabel::pauseAllMultimedia()
{
    Q_FOREACH(VideoWidget * video, videoWidgets)
        video->pause();
    Q_FOREACH(QMediaPlayer * sound, soundPlayers)
        sound->pause();
    Q_FOREACH(QMediaPlayer * sound, linkSoundPlayers)
        sound->pause();
}

bool PageLabel::hasActiveMultimediaContent() const
{
    Q_FOREACH(VideoWidget* video, videoWidgets) {
        if (video->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer* sound, soundPlayers) {
        if (sound->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer* sound, linkSoundPlayers) {
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

void PageLabel::mouseReleaseEvent(QMouseEvent* event)
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
                            Poppler::LinkExecute* link = (Poppler::LinkExecute*) links.at(i);
                            if (!link->parameters().isEmpty())
                                // It seems like I can't set and read these parameters.
                                // If anyone knows how to set these parameters in LaTeX for a Linux system, please let me know.
                                qInfo() << "Link of type execute with ignored parameters: " << link->parameters();
                            QUrl url = QUrl( link->fileName(), QUrl::TolerantMode );
                            if (!urlSplitCharacter.isEmpty()) {
                                QString fileName = url.fileName();
                                QStringList splitFileName = fileName.split(urlSplitCharacter);
                                if (splitFileName.length() > 1) {
                                    // TODO: change file name in url and use the rest as parameters
                                    if (urlSplitCharacter.contains("embed")) {
                                        qWarning() << "This is VERY EXPERIMENTAL";
                                        // Embed the executed program in a widget.
                                        // This widget should be created earlier
                                        EmbeddedWindow* embedded = EmbeddedWindow::createWindow(splitFileName.first()); // TODO: handle arguments
                                        embeddedWindows.append(embedded);
                                        embedded->setGeometry(*linkPositions.at(i));
                                        embedded->show();
                                    }

                                }
                            }
                            QDesktopServices::openUrl(url);
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
                                    qInfo() << "Unsupported link action: print.";
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
                                    qInfo() << "Unsupported link action: find.";
                                    break;
                                case Poppler::LinkAction::Presentation:
                                    // untested
                                    emit sendShowFullscreen();
                                    break;
                                case Poppler::LinkAction::EndPresentation:
                                    // untested
                                    emit sendEndFullscreen();
                                    break;
                                case Poppler::LinkAction::HistoryBack:
                                    // TODO: implement this
                                    qInfo() << "Unsupported link action: history back.";
                                    break;
                                case Poppler::LinkAction::HistoryForward:
                                    // TODO: implement this
                                    qInfo() << "Unsupported link action: history forward.";
                                    break;
                            };
                        }
                    break;
                    case Poppler::Link::Sound:
                        {
                            Poppler::LinkSound * link = (Poppler::LinkSound*) links.at(i);
                            Poppler::SoundObject * sound = link->sound();
                            if (sound->soundType() == Poppler::SoundObject::External) {
                                // This is inefficient, but usually linkSoundPlayerPositions.size()==1
                                for (int j=0; j<linkSoundPositions.size(); j++) {
                                    if (linkSoundPositions.at(j)->contains(event->pos())) {
                                        if (linkSoundPlayers.at(j)->state() == QMediaPlayer::PlayingState)
                                            linkSoundPlayers.at(j)->pause();
                                        else
                                            linkSoundPlayers.at(j)->play();
                                        break;
                                    }
                                }
                            }
                            else { // untested
                                // Most of the code in this scope is copied from stackoverflow.
                                // I have not tested it, because I don't have a pdf file with embedded sound.
                                // Sound can also be embedded as optional content, which is not supported by this PDF viewer.
                                qWarning() << "Playing embedded sound files is VERY EXPERIMENTAL.\n"
                                           << "Controling the playback is only possible with external files.";
                                QByteArray data = sound->data();
                                QBuffer * audio_buffer = new QBuffer(&data);
                                sound->soundEncoding();
                                QAudioFormat format;
                                // The function names indicate that I can translate the coding to from sound to format:
                                format.setSampleSize(sound->bitsPerSample());
                                format.setSampleRate(sound->samplingRate());
                                format.setChannelCount(sound->channels());
                                // These seem to be some default options... Just try if it works.
                                format.setCodec("audio/pcm");
                                format.setByteOrder(QAudioFormat::BigEndian);
                                switch (sound->soundEncoding())
                                {
                                    case Poppler::SoundObject::SoundEncoding::Raw:
                                        format.setSampleType(QAudioFormat::UnSignedInt);
                                        break;
                                    case Poppler::SoundObject::SoundEncoding::Signed:
                                        format.setSampleType(QAudioFormat::SignedInt);
                                        break;
                                    case Poppler::SoundObject::SoundEncoding::ALaw:
                                    case Poppler::SoundObject::SoundEncoding::muLaw:
                                        // I have no idea what this means...
                                        break;
                                }
                                QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
                                if (!info.isFormatSupported(format)) {
                                    qWarning() << "Audio format of embedded sound not supported by backend.";
                                    return;
                                }
                                QAudioOutput * output = new QAudioOutput(info, format);
                                output->start(audio_buffer);
                                connect(this, &PageLabel::slideChange, this, [&](){ delete output; delete audio_buffer; });
                            }
                        }
                    break;
                    case Poppler::Link::Movie:
                        {
                            qInfo() << "Unsupported link of type video.";
                            Poppler::LinkMovie * link = (Poppler::LinkMovie*) links.at(i);
                            Q_FOREACH(VideoWidget * video, videoWidgets) {
                                if (link->isReferencedAnnotation(video->getAnnotation()))
                                    video->play();
                            }
                        }
                    break;
                    case Poppler::Link::Rendition:
                        qInfo() << "Unsupported link of type rendition";
                    break;
                    case Poppler::Link::JavaScript:
                        qInfo() << "Unsupported link of type JavaScript";
                    break;
                    case Poppler::Link::OCGState:
                        if ( linkPositions.at(i)->contains(event->pos()) )
                            qInfo() << "Unsupported link of type OCGState";
                    break;
                    case Poppler::Link::Hide:
                        qInfo() << "Unsupported link of type hide";
                    break;
                    case Poppler::Link::None:
                        qInfo() << "Unsupported link of type none";
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

void PageLabel::mouseMoveEvent(QMouseEvent* event)
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

Poppler::Page* PageLabel::getPage()
{
    return page;
}

void PageLabel::clearCache()
{
    cachedIndex = -1;
}

void PageLabel::setPagePart(int const state)
{
    pagePart = state;
}
