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
    autostartEmbeddedTimer->setSingleShot(true);
    connect(autostartEmbeddedTimer, &QTimer::timeout, this, [&](){startAllEmbeddedApplications(pageIndex);});
    autostartTimer->setSingleShot(true);
    connect(autostartTimer, &QTimer::timeout, this, &PageLabel::startAllMultimedia);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PageLabel::timeoutSignal);
    renderPage(page, false);
}

PageLabel::PageLabel(QWidget* parent) : QLabel(parent)
{
    autostartEmbeddedTimer->setSingleShot(true);
    connect(autostartEmbeddedTimer, &QTimer::timeout, this, [&](){startAllEmbeddedApplications(pageIndex);});
    autostartTimer->setSingleShot(true);
    connect(autostartTimer, &QTimer::timeout, this, &PageLabel::startAllMultimedia);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &PageLabel::timeoutSignal);
}

PageLabel::~PageLabel()
{
    timeoutTimer->stop();
    autostartTimer->stop();
    autostartEmbeddedTimer->stop();
    clearAll();
    delete transitionWidget;
    delete timeoutTimer;
    delete autostartTimer;
    delete autostartEmbeddedTimer;
}

void PageLabel::setPresentationStatus(const bool status)
{
    isPresentation = status;
    if (isPresentation && transitionWidget==nullptr) {
        transitionWidget = new TransitionWidget(this);
        transitionWidget->show();
    }
    transitionWidget->hide();
}

void PageLabel::clearAll()
{
    // Clear all contents of the label.
    // This function is called when the document is reloaded or the program is closed and everything should be cleaned up.
    autostartTimer->stop();
    autostartEmbeddedTimer->stop();
    clearLists();
    clearCache();
    qDeleteAll(cachedVideoWidgets);
    cachedVideoWidgets.clear();
    // Clear embedded applications
    embedPositions.clear();
    qDeleteAll(embedApps);
    embedApps.clear();
    embedMap.clear();
    page = nullptr;
}

void PageLabel::clearLists()
{
    // Clear page specific content.
    // This function is called when going to an other page, which is not just an overlay of the previous page.
    // It deletes all multimedia content associated with the current page.
    qDeleteAll(videoSliders);
    videoSliders.clear();
    qDeleteAll(soundSliders);
    soundSliders.clear();
    qDeleteAll(soundLinkSliders);
    soundLinkSliders.clear();
    linkPositions.clear();
    qDeleteAll(links);
    links.clear();
    videoPositions.clear();
    qDeleteAll(videoWidgets);
    videoWidgets.clear();
    soundPositions.clear();
    qDeleteAll(soundPlayers);
    soundPlayers.clear();
    qDeleteAll(soundLinkPlayers);
    soundLinkPlayers.clear();
}

void PageLabel::renderPage(Poppler::Page* page, bool const setDuration, QPixmap const* pixmap)
{
    timeoutTimer->stop();
    if (page == nullptr)
        return;

    // Use overlay specific options
    // A page is called an overlay of the previously rendered page, if they have the same label.
    // This is also the case, if the same page is rendered again (e.g. because the window is resized).
    bool const isOverlay = this->page!=nullptr && page->label()==this->page->label();
    if (isOverlay) {
        qDeleteAll(links);
        linkPositions.clear();
        videoPositions.clear();
        soundPositions.clear();
        links.clear();
    }
    else
        clearLists();

    // Old cached images are useless if the label size has changed:
    if (size() != oldSize) {
        clearCache();
        oldSize = size();
    }

    // Set the new page and basic properties
    this->page = page;
    pageIndex = page->index();
    QSize pageSize = page->pageSize();
    // This is given in point = inch/72 ≈ 0.353mm (Did they choose these units to bother programmers?)

    // Place the page as an image of the correct size at the correct position
    // The lower left corner of the image will be located at (shift_x, shift_y)
    int shift_x=0, shift_y=0;
    int pageHeight=pageSize.height(), pageWidth=pageSize.width();
    // The page image must be split if the beamer option "notes on second screen" is set.
    if (pagePart != FullPage)
        pageWidth /= 2;
    // Check it width or height is the limiting constraint for the size of the displayed slide and calculate the resolution
    // resolution is calculated in pixels per point = dpi/72.
    if (width() * pageHeight > height() * pageWidth) {
        // the width of the label is larger than required
        resolution = double(height()) / pageHeight;
        shift_x = int(width()/2 - resolution/2 * pageWidth);
        if (isPresentation) {
            transitionWidget->setGeometry(shift_x, 0, width()-2*shift_x, height());
            transitionWidget->show();
            transitionWidget->hide();
        }
    }
    else {
        // the height of the label is larger than required
        resolution = double(width()) / pageWidth;
        shift_y = int(height()/2 - resolution/2 * pageHeight);
        if (isPresentation) {
            transitionWidget->setGeometry(0, shift_y, width(), height()-2*shift_y);
            transitionWidget->show();
            transitionWidget->hide();
        }
    }

    // Calculate the size of the image relative to the label size
    double scale_x=resolution*pageWidth, scale_y=resolution*pageHeight;
    // Adjustments if only parts of the page are shown:
    if (pagePart != FullPage) {
        scale_x *= 2;
        // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
        if (pagePart == RightHalf)
            shift_x -= width();
    }

    // Display the image
    if (pixmap != nullptr) {
        // A pixmap was passed to this function. Display this pixmap as the page image.
        if (pagePart != FullPage) {
            // The pixmap might show both notes and presentation.
            // Check the width to decide whether the image shows only the relevant part or the full page.
            QPixmap const* oldPixmap = this->pixmap();
            int referenceWidth;
            if (oldPixmap==nullptr || oldPixmap->isNull())
                referenceWidth = int(1.5*width());
            else
                referenceWidth = int(1.9*oldPixmap->width());
            if (pixmap->width() > referenceWidth) {
                // Assume that the pixmap shows notes and presentation.
                if (pagePart == LeftHalf)
                    changePixmap(pixmap->copy(0, 0, pixmap->width()/2, pixmap->height()));
                else
                    changePixmap(pixmap->copy(pixmap->width()/2, 0, pixmap->width()/2, pixmap->height()));
            }
            else
                changePixmap(*pixmap);
        }
        else
            changePixmap(*pixmap);
    }
    else if (cache.contains(pageIndex)) {
        // There exists a cached image for this page. Display this image as the page image.
        // TODO: Is this efficient? It doesn't look like...
        QPixmap const* pixmap = getCache(pageIndex);
        changePixmap(pixmap->copy());
        delete pixmap;
    }
    else {
        // A new page image has to be rendered.
        QPixmap const pixmap = getPixmap(page);
        changePixmap(pixmap);
        // Save this image to cache.
        if (useCache)
            updateCache(&pixmap, page->index());
    }

    // Show the page on the screen.
    // One could show the page in any case to make it slightly more responsive, but this can lead to a short interruption by a different image.
    // All operations before the next call to repaint() are usually very fast.
    if (!showMultimedia)
        repaint();

    // Presentation slides can have a "duration" property.
    // In this case: go to the next page after that given time.
    if (isPresentation && setDuration) {
        duration = page->duration(); // duration of the current page in s
        // For durations longer than the minimum animation delay: use the duration
        if (duration*1000 > minimumAnimationDelay) {
            timeoutTimer->start(int(1000*duration));
            if (duration < 0.5)
                repaint();
        }
        // For durations of approximately 0: use the minimum animation delay
        else if (duration > -1e-6) {
            timeoutTimer->start(minimumAnimationDelay);
            repaint();
        }
    }

    // Collect link areas in pixels (positions relative to the lower left edge of the label)
    links = page->links();
    Q_FOREACH(Poppler::Link* link, links) {
        QRectF relative = link->linkArea();
        linkPositions.append(QRect(
                    shift_x+int(relative.x()*scale_x),
                    shift_y+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                ));
    }

    // Multimedia content. This part is work in progress.
    // Execution links for embedded applications are also handled here.
    if (showMultimedia) {
        // Handle embedded applications
        for (int i=0; i<links.size(); i++) {
            if (links[i]->linkType() == Poppler::Link::Execute) {
                // Execution links can point to applications, which should be embedded in the presentation

                // First case: the execution link points to an application, which exists already as an application widget.
                // In this case the widget just needs to be shown in the correct position and size.
                // Index of the embedded widget & process:
                int idx = -1;
                if (embedMap.contains(pageIndex) && embedMap[pageIndex].contains(i))
                    idx = embedMap[pageIndex][i];
                if (idx!=-1 && embedApps[idx]->isReady()) {
                    QRect winGeometry = linkPositions[i];
                    if (winGeometry.height() < 0) {
                        winGeometry.setY(winGeometry.y() + winGeometry.height());
                        winGeometry.setHeight(-linkPositions[i].height());
                    }
                    QWidget* widget = embedApps[idx]->getWidget();
                    if (winGeometry!=embedPositions[idx]) {
                        widget->setMinimumSize(winGeometry.width(), winGeometry.height());
                        widget->setMaximumSize(winGeometry.width(), winGeometry.height());
                        widget->setGeometry(winGeometry);
                        embedPositions[idx] = winGeometry;
                    }
                    widget->show();
                }
                // Second case: There exists no process for this execution link.
                // In this case we need to check, whether this application should be executed in an embedded window.
                else if (idx==-1 || !embedApps[idx]->isStarted()) {
                    Poppler::LinkExecute* const link = static_cast<Poppler::LinkExecute*>(links[i]);
                    // Get file path (url) and arguments
                    QStringList splitFileName = QStringList();
                    if (!urlSplitCharacter.isEmpty())
                        splitFileName = link->fileName().split(urlSplitCharacter);
                    else
                        splitFileName.append(link->fileName());
                    QUrl url = QUrl(splitFileName[0], QUrl::TolerantMode);
                    splitFileName.append(link->parameters());
                    if (embedFileList.contains(splitFileName[0]) || embedFileList.contains(url.fileName()) || (splitFileName.length() > 1 && splitFileName.contains("embed"))) {
                        splitFileName.removeAll("embed"); // We know that the file will be embedded. This is not an argument for the program.
                        splitFileName.removeAll("");
                        if (embedApps.isEmpty())
                            avoidMultimediaBug();
                        QRect winGeometry = linkPositions[i];
                        if (winGeometry.height() < 0) {
                            winGeometry.setY(winGeometry.y() + winGeometry.height());
                            winGeometry.setHeight(-linkPositions[i].height());
                        }
                        if (idx == -1) {
                            bool found = false;
                            // Check if the same application exists already on an other page.
                            for (QMap<int,QMap<int,int>>::const_iterator page_it = embedMap.cbegin(); page_it!=embedMap.cend(); page_it++) {
                                for (QMap<int,int>::const_iterator idx_it = (*page_it).cbegin(); idx_it!=(*page_it).cend(); idx_it++) {
                                    if (embedApps[*idx_it]->getCommand() == splitFileName) {
                                        embedMap[pageIndex][i] = *idx_it;
                                        embedPositions[*idx_it] = winGeometry;
                                        embedApps[*idx_it]->addLocation(pageIndex, i);
                                        found = true;
                                        if (embedApps[*idx_it]->isReady()) {
                                            QWidget* widget = embedApps[*idx_it]->getWidget();
                                            widget->setMinimumSize(winGeometry.width(), winGeometry.height());
                                            widget->setMaximumSize(winGeometry.width(), winGeometry.height());
                                            widget->setGeometry(winGeometry);
                                            widget->show();
                                        }
                                        break;
                                    }
                                }
                                if (found)
                                    break;
                            }
                            if (!found) {
                                embedMap[pageIndex][i] = embedApps.length();
                                EmbedApp* const app = new EmbedApp(splitFileName, pid2wid, pageIndex, i, this);
                                connect(app, &EmbedApp::widgetReady, this, &PageLabel::receiveEmbedApp);
                                embedApps.append(app);
                                embedPositions.append(winGeometry);
                            }
                        }
                        else
                            embedPositions[idx] = winGeometry;
                    }
                }
            }
        }
        // Hide embedded widgets from other pages
        if (embedMap.contains(pageIndex)) {
            for (int i=0; i<embedApps.size(); i++) {
                if (embedApps[i]->isReady() && !embedApps[i]->isOnPage(pageIndex)) {
                    // TODO: This can lead to weird segfaults.
                    embedApps[i]->getWidget()->hide();
                }
            }
        }
        else {
            for (int i=0; i<embedApps.size(); i++) {
                if (embedApps[i]->isReady())
                    embedApps[i]->getWidget()->hide();
            }
        }

        // This can be a good point for repainting.
        // Repainting later is only reasonable if videos will be shown quickly,
        // because they have been loaded to cache, and will be started immediately.
        // When a method is reached, which can take long time, the widget will be repainted if (notRepainted==true).
        bool notRepainted = true;
        if (!cacheVideos || autostartDelay < -0.01 || autostartDelay > 0.01) {
            repaint();
            notRepainted = false;
        }

        // Handle multimedia content.
        int newSliders = 0;

        // Videos
        // Get a list of all video annotations on this page.
        QSet<Poppler::Annotation::SubType> videoType = QSet<Poppler::Annotation::SubType>();
        videoType.insert(Poppler::Annotation::AMovie);
        QList<Poppler::Annotation*> videos = page->annotations(videoType);
        // Save the positions of all video annotations and create a video widget for each of them.
        // This can take quite long and should thus be done after hiding embedded applications from other pages.
        if (videos.isEmpty()) {
            if (isOverlay) {
                qDeleteAll(videoWidgets);
                videoWidgets.clear();
                qDeleteAll(videoSliders);
                videoSliders.clear();
            }
        }
        else if (isOverlay) {
            videoWidgets.append(cachedVideoWidgets);
            cachedVideoWidgets = videoWidgets;
            videoWidgets.clear();
        }
        for (QList<Poppler::Annotation*>::const_iterator annotation=videos.cbegin(); annotation!=videos.cend(); annotation++) {
            Poppler::MovieAnnotation* video = static_cast<Poppler::MovieAnnotation*>(*annotation);
            Poppler::MovieObject* movie = video->movie();
            bool found = false;
            for (QList<VideoWidget*>::iterator widget_it=cachedVideoWidgets.begin(); widget_it!=cachedVideoWidgets.end(); widget_it++) {
                qDebug() << (*widget_it)->getUrl() << movie->url();
                if (*widget_it != nullptr && (*widget_it)->getUrl() == movie->url()) {
                    videoWidgets.append(*widget_it);
                    // Setting *widget_it to nullptr makes sure that this videoWidget will not be deleted when cleaning up oldVideos.
                    *widget_it = nullptr;
                    found = true;
                    break;
                }
            }
            QRectF relative = video->boundary();
            videoPositions.append(QRect(
                    shift_x+int(relative.x()*scale_x),
                    shift_y+int(relative.y()*scale_y),
                    int(relative.width()*scale_x),
                    int(relative.height()*scale_y)
                ));
            if (found)
                delete video;
            else {
                if (notRepainted) {
                    repaint();
                    notRepainted = false;
                }
                qDebug() << "Loading new video widget:" << movie->url();
                videoWidgets.append(new VideoWidget(video, urlSplitCharacter, this));
            }
            newSliders++;
        }
        // Clean up old video widgets and sliders:
        for (int i=0; i<cachedVideoWidgets.size(); i++) {
            if (cachedVideoWidgets[i]!=nullptr) {
                // This cached video widget was useless and gets deleted.
                delete cachedVideoWidgets[i];
                if (videoSliders.contains(i)) {
                    delete videoSliders[i];
                    videoSliders.remove(i);
                }
            }
            else if (videoSliders.contains(i))
                // If we continue using a video widget, which already has a slider (because it is
                // in an overlay), we need one new slider less.
                newSliders--;
        }
        cachedVideoWidgets.clear();
        // The list "videos" is cleaned, but its items (annotation pointers) are not deleted! The video widgets take ownership of the annotations.
        videos.clear();

        // Sound links
        QList<QMediaPlayer*> oldSoundLinks;
        if (isOverlay) {
            oldSoundLinks = soundLinkPlayers.values();
            soundLinkPlayers.clear();
        }
        for (int i=0; i<links.size(); i++) {
            if (links[i]->linkType() == Poppler::Link::Sound) {
                // This can take relatively long. Repainting here is usually reasonable.
                if (notRepainted) {
                    repaint();
                    notRepainted = false;
                }
                // Audio links
                Poppler::SoundObject* sound = static_cast<Poppler::LinkSound*>(links[i])->sound();
                if (sound->soundType() == Poppler::SoundObject::Embedded) {
                    qWarning() << "Embedded sound files are not supported.";
                    break;
                }
                QUrl url = QUrl(sound->url(), QUrl::TolerantMode);
                QStringList splitFileName = QStringList();
                // TODO: test this
                if (!urlSplitCharacter.isEmpty()) {
                    splitFileName = sound->url().split(urlSplitCharacter);
                    url = QUrl(splitFileName[0], QUrl::TolerantMode);
                    splitFileName.pop_front();
                }
                if (!url.isValid())
                    url = QUrl::fromLocalFile(url.path());
                if (url.isRelative())
                    url = QUrl::fromLocalFile(QDir(".").absoluteFilePath(url.path()));
                if (isOverlay && !oldSoundLinks.isEmpty()) {
                    bool found=false;
                    for (QList<QMediaPlayer*>::iterator player_it=oldSoundLinks.begin(); player_it!=oldSoundLinks.end(); player_it++) {
                        QMediaContent media = (*player_it)->media();
                        // TODO: reliable check if the media names match
                        if (*player_it != nullptr && !media.isNull() && media.canonicalUrl()==url) {
                            soundLinkPlayers[i]= *player_it;
                            *player_it = nullptr;
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        break;
                }
                // If no player was found, create a new one.
                QMediaPlayer* player = new QMediaPlayer(this, QMediaPlayer::LowLatency);
                player->setMedia(url);
                soundLinkPlayers[i] = player;
                newSliders++;
                // Untested!
                if (splitFileName.contains("loop")) {
                    qDebug() << "Using untested option loop for sound";
                    connect(player, &QMediaPlayer::mediaStatusChanged, player, [&](QMediaPlayer::MediaStatus const status){if(status==QMediaPlayer::EndOfMedia) player->play();});
                }
                if (splitFileName.contains("autostart")) {
                    qDebug() << "Using untested option autostart for sound";
                    player->play();
                }
            }
        }
        // Clean up old sound link players and sliders:
        for (int i=0; i<oldSoundLinks.size(); i++) {
            if (oldSoundLinks[i]!=nullptr) {
                delete oldSoundLinks[i];
                if (soundLinkSliders.contains(i)) {
                    delete soundLinkSliders[i];
                    soundLinkSliders.remove(i);
                }
                else
                    qDebug() << "No slider found: page" << pageIndex << "old sound index" << i;
            }
        }

        // Audio as annotations (Untested, I don't know whether this is useful for anything)
        // Get a list of all audio annotations on this page.
        QSet<Poppler::Annotation::SubType> soundType = QSet<Poppler::Annotation::SubType>();
        soundType.insert(Poppler::Annotation::ASound);
        QList<Poppler::Annotation*> sounds = page->annotations(soundType);
        // Save the positions of all audio annotations and create a sound player for each of them.
        if (sounds.isEmpty()) {
            if (isOverlay) {
                qDeleteAll(soundPlayers);
                soundPlayers.clear();
                qDeleteAll(soundSliders);
                soundSliders.clear();
            }
        }
        else if (isOverlay && !soundPlayers.isEmpty()) {
            if (notRepainted) {
                repaint();
                notRepainted = false;
            }
            // Untested!
            // TODO: Make sure that things get deleted if necessary!
            QList<QMediaPlayer*> oldSounds = soundPlayers;
            soundPlayers.clear();
            for (QList<Poppler::Annotation*>::const_iterator annotation=sounds.cbegin(); annotation!=sounds.cend(); annotation++) {
                Poppler::SoundObject* sound = static_cast<Poppler::SoundAnnotation*>(*annotation)->sound();
                bool found=false;
                QUrl url = QUrl(sound->url(), QUrl::TolerantMode);
                QStringList splitFileName = QStringList();
                // Get file path (url) and arguments
                // TODO: test this
                if (!urlSplitCharacter.isEmpty()) {
                    splitFileName = sound->url().split(urlSplitCharacter);
                    url = QUrl(splitFileName[0], QUrl::TolerantMode);
                    splitFileName.pop_front();
                }
                if (!url.isValid())
                    url = QUrl::fromLocalFile(url.path());
                if (url.isRelative())
                    url = QUrl::fromLocalFile(QDir(".").absoluteFilePath(url.path()));
                for (QList<QMediaPlayer*>::iterator player_it=oldSounds.begin(); player_it!=oldSounds.end(); player_it++) {
                    QMediaContent media = (*player_it)->media();
                    // TODO: reliable check if the media names match
                    if (*player_it != nullptr && !media.isNull() && media.canonicalUrl()==url) {
                        soundPlayers.append(*player_it);
                        *player_it = nullptr;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    QMediaPlayer* player = new QMediaPlayer(this, QMediaPlayer::LowLatency);
                    player->setMedia(url);
                    // Untested!
                    if (splitFileName.contains("loop")) {
                        qDebug() << "Using untested option loop for sound";
                        connect(player, &QMediaPlayer::mediaStatusChanged, player, [&](QMediaPlayer::MediaStatus const status){if(status==QMediaPlayer::EndOfMedia) player->play();});
                    }
                    if (splitFileName.contains("autostart")) {
                        qDebug() << "Using untested option autostart for sound";
                        player->play();
                    }
                    soundPlayers.append(player);
                    newSliders++;
                }
                QRectF relative = (*annotation)->boundary();
                videoPositions.append(QRect(
                        shift_x+int(relative.x()*scale_x),
                        shift_y+int(relative.y()*scale_y),
                        int(relative.width()*scale_x),
                        int(relative.height()*scale_y)
                    ));
            }
            // Clean up old sound players and sliders:
            for (int i=0; i<oldSounds.size(); i++) {
                if (oldSounds[i]!=nullptr) {
                    delete oldSounds[i];
                    if (soundSliders.contains(i)) {
                        delete soundSliders[i];
                        soundSliders.remove(i);
                    }
                    else
                        qDebug() << "No slider found: page" << pageIndex << "old sound index" << i;
                }
            }
        }
        else {
            if (notRepainted) {
                repaint();
                notRepainted = false;
            }
            for (QList<Poppler::Annotation*>::const_iterator it = sounds.cbegin(); it!=sounds.cend(); it++) {
                qWarning() << "Support for sound in annotations is untested!";
                {
                    QRectF relative = (*it)->boundary();
                    soundPositions.append(QRect(
                                shift_x+int(relative.x()*scale_x),
                                shift_y+int(relative.y()*scale_y),
                                int(relative.width()*scale_x),
                                int(relative.height()*scale_y)
                            ));
                }

                Poppler::SoundObject* sound = static_cast<Poppler::SoundAnnotation*>(*it)->sound();
                QMediaPlayer* player = new QMediaPlayer(this, QMediaPlayer::LowLatency);
                QUrl url = QUrl(sound->url(), QUrl::TolerantMode);
                QStringList splitFileName = QStringList();
                // Get file path (url) and arguments
                // TODO: test this
                if (!urlSplitCharacter.isEmpty()) {
                    splitFileName = sound->url().split(urlSplitCharacter);
                    url = QUrl(splitFileName[0], QUrl::TolerantMode);
                    splitFileName.pop_front();
                }
                if (!url.isValid())
                    url = QUrl::fromLocalFile(url.path());
                if (url.isRelative())
                    url = QUrl::fromLocalFile(QDir(".").absoluteFilePath(url.path()));
                player->setMedia(url);
                // Untested!
                if (splitFileName.contains("loop")) {
                    qDebug() << "Using untested option loop for sound";
                    connect(player, &QMediaPlayer::mediaStatusChanged, player, [&](QMediaPlayer::MediaStatus const status){if(status==QMediaPlayer::EndOfMedia) player->play();});
                }
                if (splitFileName.contains("autostart")) {
                    qDebug() << "Using untested option autostart for sound";
                    player->play();
                }
                soundPlayers.append(player);
                newSliders++;
            }
        }
        qDeleteAll(sounds);
        sounds.clear();

        // Autostart video widgets if the option is set as arguments in the video annotation in the pdf
        for (int i=0; i<videoWidgets.size(); i++) {
            if (videoWidgets[i]->getAutoplay()) {
                qDebug() << "Untested option autostart for video";
                videoWidgets[i]->setGeometry(videoPositions[i]);
                videoWidgets[i]->show();
                videoWidgets[i]->play();
            }
        }
        // Autostart multimedia if the option is set in BeamerPresenter
        if (videoWidgets.size() + soundPlayers.size() + soundLinkPlayers.size() != 0) {
            if (autostartDelay > 0.01)
                // autostart with delay
                autostartTimer->start(int(autostartDelay*1000));
            else if (autostartDelay > -0.01)
                // autostart without delay
                startAllMultimedia();
        }
        if (notRepainted)
            repaint();

        // Autostart embedded applications if the option is set in BeamerPresenter
        if (embedMap.contains(pageIndex)) {
            if (autostartEmbeddedDelay > 0.01)
                // autostart with delay
                autostartEmbeddedTimer->start(int(autostartEmbeddedDelay*1000));
            else if (autostartEmbeddedDelay > -0.01)
                // autostart without delay
                startAllEmbeddedApplications(pageIndex);
        }

        // Add sliders
        if (newSliders!=0)
            emit requestMultimediaSliders(newSliders);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Cache management and image rendering
////////////////////////////////////////////////////////////////////////////////////////////////////

void PageLabel::changePixmap(const QPixmap pixmap)
{
    // Set pixmap of this.
    // If this is the presentation label, show slide transitions.
    if (isPresentation) {
        if (page->transition() != nullptr) {
            transitionWidget->shiftImages(pixmap);
            transitionWidget->animate(page->transition());
        }
        else
            transitionWidget->hide();
    }
    setPixmap(pixmap);
}

void PageLabel::updateCacheVideos(const Poppler::Page *page)
{
    if (page->index()==this->pageIndex)
        return;
    // Get a list of all video annotations on this page.
    QSet<Poppler::Annotation::SubType> videoType = QSet<Poppler::Annotation::SubType>();
    videoType.insert(Poppler::Annotation::AMovie);
    QList<Poppler::Annotation*> videos = page->annotations(videoType);
    for (QList<Poppler::Annotation*>::const_iterator annotation=videos.cbegin(); annotation!=videos.cend(); annotation++) {
        Poppler::MovieAnnotation* video = static_cast<Poppler::MovieAnnotation*>(*annotation);
        Poppler::MovieObject* movie = video->movie();
        bool found = false;
        for (QList<VideoWidget*>::iterator widget_it=cachedVideoWidgets.begin(); widget_it!=cachedVideoWidgets.end(); widget_it++) {
            if (*widget_it != nullptr && (*widget_it)->getUrl() == movie->url()) {
                found = true;
                break;
            }
        }
        if (found)
            delete video;
        else {
            qDebug() << "Cache new video widget:" << movie->url();
            cachedVideoWidgets.append(new VideoWidget(video, urlSplitCharacter, this));
        }
    }
    videos.clear();
}

long int PageLabel::updateCache(QPixmap const* pixmap, int const index)
{
    // Save the pixmap to (compressed) cache of page index and return the size of the compressed image.
    if (pixmap==nullptr || pixmap->isNull())
        return 0;
    // The image will be compressed and written to a QByteArray.
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    pixmap->save(&buffer, "PNG");
    cache[index] = bytes;
    return bytes->size();
}

long int PageLabel::updateCache(QByteArray const* bytes, int const index)
{
    // Write bytes to the cache of page index and return the size of bytes.
    if (bytes==nullptr || bytes->isNull() || bytes->isEmpty())
        return 0;
    else if (cache.contains(index))
        delete cache[index];
    cache[index] = bytes;
    return bytes->size();
}

long int PageLabel::updateCache(Poppler::Page const* cachePage)
{
    // Check whether the cachePage exists in cache. If yes, return 0.
    // Otherwise, render the given page using the internal renderer,
    // write the compressed image to cache and return the size of the compressed image.

    int index = cachePage->index();
    // Check whether the page exists in cache.
    if (cache.contains(index))
        return 0;

    // Render the page to a pixmap
    QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
    // if pagePart != FullPage: Reduce the image to the relevant part.
    if (pagePart == LeftHalf)
        image = image.copy(0, 0, image.width()/2, image.height());
    else if (pagePart == RightHalf)
        image = image.copy(image.width()/2, 0, image.width()/2, image.height());

    // This check is repeated, because it could be possible that the cache is overwritten while the image is rendered.
    if (cache.contains(index))
        return 0;

    // Write the image in png format to a QBytesArray
    QByteArray* bytes = new QByteArray();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    cache[index] = bytes;
    return bytes->size();
}

QPixmap PageLabel::getPixmap(Poppler::Page const* cachePage) const
{
    // Return a pixmap representing the current page.
    QPixmap pixmap;
    if (cache.contains(cachePage->index())) {
        // The page exists in cache. Use the cache instead of rendering it again.
        QPixmap const* pixpointer = getCache(cachePage->index());
        pixmap = *pixpointer;
        delete pixpointer;
    }
    else if (pagePart == FullPage)
        pixmap = QPixmap::fromImage(cachePage->renderToImage(72*resolution, 72*resolution));
    else {
        QImage image = cachePage->renderToImage(72*resolution, 72*resolution);
        if (pagePart == LeftHalf)
            pixmap = QPixmap::fromImage(image.copy(0, 0, image.width()/2, image.height()));
        else
            pixmap = QPixmap::fromImage(image.copy(image.width()/2, 0, image.width()/2, image.height()));
    }
    return pixmap;
}

QPixmap const* PageLabel::getCache(int const index) const
{
    // Get a pixmap from cache.
    QPixmap* pixmap = new QPixmap();
    if (cache.contains(index)) {
        pixmap->loadFromData(*cache[index], "PNG");
        // If an external renderer is used, cached images always show the full page.
        // But if pagePart != FullPage, only one half of the image should be shown.
        if (pagePart != FullPage) {
            // The cached pixmap might show both notes and presentation.
            // Check the width to decide whether the image shows only the relevant part or the full page.
            int referenceWidth;
            if (this->pixmap()==nullptr || this->pixmap()->isNull())
                referenceWidth = int(1.5*width());
            else
                referenceWidth = int(1.9*this->pixmap()->width());
            if (pixmap->width() > referenceWidth) {
                // Assume that the pixmap shows notes and presentation.
                QPixmap* oldpixmap = pixmap;
                if (pagePart == LeftHalf)
                    pixmap = new QPixmap(pixmap->copy(0, 0, pixmap->width()/2, pixmap->height()));
                else
                    pixmap = new QPixmap(pixmap->copy(pixmap->width()/2, 0, pixmap->width()/2, pixmap->height()));
                delete oldpixmap;
            }
        }
    }
    return pixmap;
}

QByteArray const* PageLabel::getCachedBytes(int const index) const
{
    if (cache.contains(index))
        return cache[index];
    else
        return new QByteArray();
}

long int PageLabel::getCacheSize() const
{
    // Return the total size of all cached images of this label in bytes.
    long int size=0;
    for (QMap<int,QByteArray const*>::const_iterator it=cache.cbegin(); it!=cache.cend(); it++) {
        size += (*it)->size();
    }
    return size;
}

void PageLabel::clearCache()
{
    // Remove all images from cache.
    for (QMap<int,QByteArray const*>::const_iterator bytes=cache.cbegin(); bytes!=cache.cend(); bytes++) {
        delete *bytes;
    }
    cache.clear();
}

long int PageLabel::clearCachePage(const int index)
{
    // Delete the given page (page number index+1) from cache and return its size.
    // Return 0 if the page does not exist in cache.
    if (cache.contains(index)) {
        long int size = cache[index]->size();
        delete cache[index];
        cache.remove(index);
        return size;
    }
    else
        return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Multimedia
////////////////////////////////////////////////////////////////////////////////////////////////////
void PageLabel::setMultimediaSliders(QList<QSlider*> sliderList)
{
    // Connect multimedia content of the current slide to the given sliders.
    // this takes ownership of the items of sliderList.
    if (videoSliders.size() + soundSliders.size() + soundLinkSliders.size() + sliderList.size() != videoWidgets.size() + soundLinkPlayers.size() + soundPlayers.size()) {
        qCritical() << "Something unexpected happened: There is a problem with the media sliders.";
        qDebug() << "videos" << videoWidgets.size() << videoSliders.size() << "sound links" << soundLinkPlayers.size() << soundLinkSliders.size() << "sounds" << soundPlayers.size() << soundSliders.size() << "new sliders" << sliderList.size();
        return;
    }
    // TODO: better multimedia controls
    QList<QSlider*>::const_iterator slider = sliderList.cbegin();
    for (int i=0; i<videoWidgets.size(); i++) {
        if (!videoSliders.contains(i)) {
            (*slider)->setRange(0, int(videoWidgets[i]->getDuration()));
            connect(*slider, &QAbstractSlider::sliderMoved, videoWidgets[i]->getPlayer(), &QMediaPlayer::setPosition);
            connect(videoWidgets[i]->getPlayer(), &QMediaPlayer::positionChanged, *slider, &QSlider::setValue);
            connect(videoWidgets[i]->getPlayer(), &QMediaPlayer::durationChanged, *slider, &QSlider::setMaximum);
            videoSliders[i] = *slider;
            slider++;
        }
    }
    for (QMap<int,QMediaPlayer*>::const_iterator it=soundLinkPlayers.cbegin(); it!=soundLinkPlayers.cend(); it++, slider++) {
        if (!soundLinkSliders.contains(it.key())) {
            (*slider)->setRange(0, int((*it)->duration()));
            connect(*slider, &QAbstractSlider::sliderMoved, *it, &QMediaPlayer::setPosition);
            connect(*it, &QMediaPlayer::positionChanged, *slider, &QSlider::setValue);
            connect(*it, &QMediaPlayer::durationChanged, *slider, &QSlider::setMaximum);
            soundLinkSliders[it.key()] = *slider;
            slider++;
        }
    }
    for (int i=0; i<soundPlayers.size(); i++) {
        if (!soundSliders.contains(i)) {
            (*slider)->setRange(0, int(soundPlayers[i]->duration()));
            connect(*slider, &QAbstractSlider::sliderMoved, soundPlayers[i], &QMediaPlayer::setPosition);
            connect(soundPlayers[i], &QMediaPlayer::positionChanged, *slider, &QSlider::setValue);
            connect(soundPlayers[i], &QMediaPlayer::durationChanged, *slider, &QSlider::setMaximum);
            soundSliders[i] = *slider;
            slider++;
        }
    }
}

void PageLabel::startAllMultimedia()
{
    for (int i=0; i<videoWidgets.size(); i++) {
        // The size of a video widget is set the first time it gets shown.
        // Setting this directly at initialization caused some problems.
        videoWidgets[i]->setGeometry(videoPositions[i]);
        videoWidgets[i]->show();
        videoWidgets[i]->play();
    }
    Q_FOREACH(QMediaPlayer* sound, soundPlayers)
        sound->play();
    Q_FOREACH(QMediaPlayer* sound, soundLinkPlayers)
        sound->play();
}

void PageLabel::pauseAllMultimedia()
{
    Q_FOREACH(VideoWidget* video, videoWidgets)
        video->pause();
    Q_FOREACH(QMediaPlayer* sound, soundPlayers)
        sound->pause();
    Q_FOREACH(QMediaPlayer* sound, soundLinkPlayers)
        sound->pause();
}

bool PageLabel::hasActiveMultimediaContent() const
{
    // Return true if any multimedia content is currently being played
    Q_FOREACH(VideoWidget* video, videoWidgets) {
        if (video->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer* sound, soundPlayers) {
        if (sound->state() == QMediaPlayer::PlayingState)
            return true;
    }
    Q_FOREACH(QMediaPlayer* sound, soundLinkPlayers) {
        if (sound->state() == QMediaPlayer::PlayingState)
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Mouse events
////////////////////////////////////////////////////////////////////////////////////////////////////
void PageLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        for (int i=0; i<links.size(); i++) {
            if (linkPositions[i].contains(event->pos())) {
                switch ( links[i]->linkType() )
                {
                    case Poppler::Link::Goto:
                        if (static_cast<Poppler::LinkGoto*>(links[i])->isExternal()) {
                            // Link to an other document
                            QString filename = static_cast<Poppler::LinkGoto*>(links[i])->fileName();
                            QDesktopServices::openUrl(QUrl(filename, QUrl::TolerantMode));
                        }
                        else {
                            // Link to an other page
                            emit sendNewPageNumber( static_cast<Poppler::LinkGoto*>(links[i])->destination().pageNumber() - 1 );
                        }
                        return;
                    case Poppler::Link::Execute:
                        // Handle execution links, which are marked for execution as an embedded application.
                        // In this case, a corresponding item has been added to embeddedWidgets in renderPage.
                        if (embedMap.contains(pageIndex) && embedMap[pageIndex].contains(i)) {
                            int const idx = embedMap[pageIndex][i];
                            // First case: the execution link points to an application, which exists already as an application widget.
                            // In this case the widget just needs to be shown in the correct position and size.
                            if (embedApps[idx]->isReady()) {
                                QRect const* winGeometry = &embedPositions[idx];
                                QWidget* widget = embedApps[idx]->getWidget();
                                widget->setMinimumSize(winGeometry->width(), winGeometry->height());
                                widget->setMaximumSize(winGeometry->width(), winGeometry->height());
                                widget->setGeometry(*winGeometry);
                                widget->show();
                                break;
                            }
                            // Second case: There exists no process for this execution link.
                            // In this case we need to check, whether this application should be executed in an embedded window.
                            embedApps[idx]->start();
                            break;
                        }
                        // Execution links not marked for embedding are handed to the desktop services.
                        else {
                            Poppler::LinkExecute* link = static_cast<Poppler::LinkExecute*>(links[i]);
                            QStringList splitFileName = QStringList();
                            if (!urlSplitCharacter.isEmpty())
                                splitFileName = link->fileName().split(urlSplitCharacter);
                            else
                                splitFileName.append(link->fileName());
                            QUrl url = QUrl(splitFileName[0], QUrl::TolerantMode);
                            // TODO: handle arguments
                            QDesktopServices::openUrl(url);
                        }
                        break;
                    case Poppler::Link::Browse:
                        // Link to file or website
                        QDesktopServices::openUrl( QUrl(static_cast<Poppler::LinkBrowse*>(links[i])->url(), QUrl::TolerantMode) );
                        break;
                    case Poppler::Link::Action:
                        {
                            Poppler::LinkAction* link = static_cast<Poppler::LinkAction*>(links[i]);
                            switch (link->actionType())
                            {
                                case Poppler::LinkAction::Quit:
                                case Poppler::LinkAction::Close:
                                    emit sendCloseSignal();
                                    return;
                                case Poppler::LinkAction::Print:
                                    qInfo() << "Unsupported link action: print.";
                                    break;
                                case Poppler::LinkAction::GoToPage:
                                    emit focusPageNumberEdit();
                                    break;
                                case Poppler::LinkAction::PageNext:
                                    emit sendNewPageNumber(pageIndex + 1);
                                    return;
                                case Poppler::LinkAction::PagePrev:
                                    emit sendNewPageNumber(pageIndex - 1);
                                    return;
                                case Poppler::LinkAction::PageFirst:
                                    emit sendNewPageNumber(0);
                                    return;
                                case Poppler::LinkAction::PageLast:
                                    emit sendNewPageNumber(-1);
                                    return;
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
                            }
                        }
                        break;
                    case Poppler::Link::Sound:
                        {
                            Poppler::LinkSound* link = static_cast<Poppler::LinkSound*>(links[i]);
                            Poppler::SoundObject* sound = link->sound();
                            if (sound->soundType() == Poppler::SoundObject::External) {
                                if (soundLinkPlayers[i]->state() == QMediaPlayer::PlayingState)
                                    soundLinkPlayers[i]->pause();
                                else
                                    soundLinkPlayers[i]->play();
                            }
                            else
                                qWarning() << "Playing embedded sound files is not supported.";
                        }
                        break;
                    case Poppler::Link::Movie:
                        {
                            qInfo() << "Unsupported link of type video. If this works, you should be surprised.";
                            // I don't know if the following lines make any sense.
                            Poppler::LinkMovie* link = static_cast<Poppler::LinkMovie*>(links[i]);
                            Q_FOREACH(VideoWidget* video, videoWidgets) {
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
            if (soundPositions[i].contains(event->pos())) {
                if (soundPlayers[i]->state() == QMediaPlayer::PlayingState)
                    soundPlayers[i]->pause();
                else
                    soundPlayers[i]->play();
            }
        }
        for (int i=0; i<videoPositions.size(); i++) {
            if (videoPositions[i].contains(event->pos())) {
                if (videoWidgets[i]->state() == QMediaPlayer::PlayingState)
                    videoWidgets[i]->pause();
                else {
                    videoWidgets[i]->setGeometry(videoPositions[i]);
                    videoWidgets[i]->show();
                    videoWidgets[i]->play();
                }
                return;
            }
        }
    }
    event->accept();
}

void PageLabel::mouseMoveEvent(QMouseEvent* event)
{
    // Show the cursor as Qt::PointingHandCursor when hoovering links
    if (!pointer_visible)
        return;
    bool is_arrow_pointer = cursor() == Qt::ArrowCursor;
    for (QList<QRect>::const_iterator pos_it=linkPositions.cbegin(); pos_it!=linkPositions.cend(); pos_it++) {
        if (pos_it->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    for (QList<QRect>::const_iterator pos_it=soundPositions.cbegin(); pos_it!=soundPositions.cend(); pos_it++) {
        if (pos_it->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    for (QList<QRect>::const_iterator pos_it=videoPositions.cbegin(); pos_it!=videoPositions.cend(); pos_it++) {
        if (pos_it->contains(event->pos())) {
            if (is_arrow_pointer)
                setCursor(Qt::PointingHandCursor);
            return;
        }
    }
    if (!is_arrow_pointer)
        setCursor(Qt::ArrowCursor);
    event->accept();
}

void PageLabel::togglePointerVisibility()
{
    if (pointer_visible) {
        pointer_visible = false;
        setMouseTracking(false);
        setCursor(Qt::BlankCursor);
    }
    else {
        pointer_visible = true;
        setMouseTracking(true);
        setCursor(Qt::ArrowCursor);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//// Embedded applications
////////////////////////////////////////////////////////////////////////////////////////////////////
void PageLabel::receiveEmbedApp(EmbedApp* app)
{
    // Geometry of the embedded window:
    int const*const location = app->getNextLocation(pageIndex);
    int const idx = embedMap[location[0]][location[1]];
    QRect const*const winGeometry = &embedPositions[idx];
    // Turn the window into a widget, which can be embedded in the presentation (or control) window:
    QWidget* const widget = app->getWidget();
    widget->setMinimumSize(winGeometry->width(), winGeometry->height());
    widget->setMaximumSize(winGeometry->width(), winGeometry->height());
    // Showing and hiding the widget here if page!=pageIndex makes showing the widget faster.
    widget->setGeometry(*winGeometry);
    widget->show();
    if (location[0]!=pageIndex)
        widget->hide();
}

void PageLabel::initEmbeddedApplications(Poppler::Page const* page)
{
    // Initialize all embedded applications for a given page.
    // The applications are not started yet, but their positions are calculated and the commands are saved.
    // After this function, PageLabel::startAllEmbeddedApplications can be used to start the applications.
    QList<Poppler::Link*> links;
    int const index = page->index();
    if (index == pageIndex)
        links = this->links;
    else
        links = page->links();
    bool containsNewEmbeddedWidgets = false;

    // Find embedded programs.
    for (int i=0; i<links.length(); i++) {
        if (links[i]->linkType()==Poppler::Link::Execute && !(embedMap.contains(index) && embedMap[index].contains(i))) {
            // Execution links can point to applications, which should be embedded in the presentation
            Poppler::LinkExecute* const link = static_cast<Poppler::LinkExecute*>(links[i]);
            // Get file path (url) and arguments
            QStringList splitFileName = QStringList();
            if (!urlSplitCharacter.isEmpty())
                splitFileName = link->fileName().split(urlSplitCharacter);
            else
                splitFileName.append(link->fileName());
            QUrl url = QUrl(splitFileName[0], QUrl::TolerantMode);
            splitFileName.append(link->parameters());
            if (embedFileList.contains(splitFileName[0]) || embedFileList.contains(url.fileName()) || (splitFileName.length() > 1 && splitFileName.contains("embed"))) {
                splitFileName.removeAll("embed"); // We know that the file will be embedded. This is not an argument for the program.
                splitFileName.removeAll("");
                if (embedApps.isEmpty())
                    avoidMultimediaBug();
                bool found = false;
                // Check if the same application exists already on an other page.
                for (QMap<int,QMap<int,int>>::const_iterator page_it = embedMap.cbegin(); page_it!=embedMap.cend(); page_it++) {
                    for (QMap<int,int>::const_iterator idx_it = (*page_it).cbegin(); idx_it!=(*page_it).cend(); idx_it++) {
                        if (embedApps[*idx_it]->getCommand() == splitFileName) {
                            embedMap[index][i] = *idx_it;
                            embedPositions[*idx_it] = QRect();
                            embedApps[*idx_it]->addLocation(index, i);
                            found = true;
                            break;
                        }
                    }
                    if (found)
                        break;
                }
                if (!found) {
                    embedMap[index][i] = embedApps.length();
                    EmbedApp* const app = new EmbedApp(splitFileName, pid2wid, index, i, this);
                    connect(app, &EmbedApp::widgetReady, this, &PageLabel::receiveEmbedApp);
                    embedApps.append(app);
                    embedPositions.append(QRect());
                }
                containsNewEmbeddedWidgets = true;
            }
        }
    }

    // If this slide contains embedded applications, calculate and save their position.
    if (containsNewEmbeddedWidgets) {
        if (index == pageIndex) {
            for (QMap<int,int>::const_iterator idx_it=embedMap[index].cbegin(); idx_it!=embedMap[index].cend(); idx_it++) {
                if (embedPositions[*idx_it].isNull()) {
                    QRect winGeometry = linkPositions[idx_it.key()];
                    if (winGeometry.height() < 0) {
                        winGeometry.setY(winGeometry.y() + winGeometry.height());
                        winGeometry.setHeight(-linkPositions[idx_it.key()].height());
                    }
                    embedPositions[*idx_it] = winGeometry;
                    if (embedApps[*idx_it]->isReady()) {
                        QWidget* const widget = embedApps[*idx_it]->getWidget();
                        widget->setMinimumSize(winGeometry.width(), winGeometry.height());
                        widget->setMaximumSize(winGeometry.width(), winGeometry.height());
                        widget->setGeometry(winGeometry);
                        widget->show();
                    }
                }
            }
        }
        else {
            int shift_x=0, shift_y=0;
            double resolution = this->resolution;
            QSize pageSize = page->pageSize();
            // This is given in point = inch/72 ≈ 0.353mm (Did they choose these units to bother programmers?)

            // Place the page as an image of the correct size at the correct position
            // The lower left corner of the image will be located at (shift_x, shift_y)
            int pageHeight=pageSize.height(), pageWidth=pageSize.width();
            // The page image must be split if the beamer option "notes on second screen" is set.
            if (pagePart != FullPage)
                pageWidth /= 2;
            // Check it width or height is the limiting constraint for the size of the displayed slide and calculate the resolution
            // resolution is calculated in pixels per point = dpi/72.
            if (width() * pageHeight > height() * pageWidth) {
                // the width of the label is larger than required
                resolution = double(height()) / pageHeight;
                shift_x = int(width()/2 - resolution/2 * pageWidth);
            }
            else {
                // the height of the label is larger than required
                resolution = double(width()) / pageWidth;
                shift_y = int(height()/2 - resolution/2 * pageHeight);
            }

            // Calculate the size of the image relative to the label size
            double scale_x=resolution*pageWidth, scale_y=resolution*pageHeight;
            // Adjustments if only parts of the page are shown:
            if (pagePart != FullPage) {
                scale_x *= 2;
                // If only the right half of the page will be shown, the position of the page (relevant for link positions) must be adjusted.
                if (pagePart == RightHalf)
                    shift_x -= width();
            }
            for (QMap<int,int>::const_iterator idx_it=embedMap[index].cbegin(); idx_it!=embedMap[index].cend(); idx_it++) {
                if (embedPositions[*idx_it].isNull()) {
                    QRectF relative = links[idx_it.key()]->linkArea();
                    QRect winGeometry = QRect(
                                shift_x+int(relative.x()*scale_x),
                                shift_y+int(relative.y()*scale_y),
                                int(relative.width()*scale_x),
                                int(relative.height()*scale_y)
                            );
                    if (winGeometry.height() < 0) {
                        int const height = -winGeometry.height();
                        winGeometry.setY(winGeometry.y() + winGeometry.height());
                        winGeometry.setHeight(height);
                    }
                    embedPositions[*idx_it] = winGeometry;
                }
            }
        }
    }

    // If the links were not stolen from the page, they should be deleted.
    if (index != pageIndex)
        qDeleteAll(links);
}

void PageLabel::startAllEmbeddedApplications(int const index)
{
    // Start all embedded applications of the given slide (slide number = index)
    if (!embedMap.contains(index))
        return;
    for(QMap<int,int>::const_iterator idx_it=embedMap[index].cbegin(); idx_it!=embedMap[index].cend(); idx_it++)
        embedApps[*idx_it]->start();
}

void PageLabel::avoidMultimediaBug()
{
    // TODO: find a better way to avoid this problem
    // This is a very ugly and inefficient way of avoiding compatibility problems of combining videos and embedded applications.
    // Probably the strange behavior without this function is caused by unconventional handling of external windows.
    // I don't know what problems occure on platforms other than GNU/Linux!
    QVideoWidget* dummy = new QVideoWidget(this);
    QMediaPlayer* dummy_player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    dummy_player->setVideoOutput(dummy);
    delete dummy_player;
    delete dummy;
}
