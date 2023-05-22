// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QtConfig>
#include <algorithm>
#include <QTimerEvent>
#include <QString>
#include <QThread>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QSizeF>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "src/log.h"
#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"
#include "src/slideview.h"
#include "src/drawing/tool.h"
#include "src/gui/clockwidget.h"
#include "src/gui/analogclockwidget.h"
#include "src/gui/tabwidget.h"
#include "src/gui/stackedwidget.h"
#include "src/gui/containerwidget.h"
#include "src/gui/slidenumberwidget.h"
#include "src/gui/slidelabelwidget.h"
#include "src/gui/settingswidget.h"
#include "src/gui/timerwidget.h"
#include "src/gui/noteswidget.h"
#include "src/gui/tocwidget.h"
#include "src/gui/thumbnailwidget.h"
#include "src/gui/toolselectorwidget.h"
#include "src/gui/toolwidget.h"
#include "src/gui/searchwidget.h"
#include "src/rendering/pixcache.h"
#include "src/names.h"
#include "src/preferences.h"

Master::Master()
{
    connect(preferences(), &Preferences::sendErrorMessage, this, &Master::showErrorMessage);
}

Master::~Master()
{
    emit clearCache();
    for (const auto cache : qAsConst(caches))
        cache->thread()->quit();
    for (const auto cache : qAsConst(caches))
    {
        cache->thread()->wait(10000);
        delete cache;
    }
    for (const auto doc : qAsConst(documents))
    {
        QList<SlideScene*> &scenes = doc->getScenes();
        while (!scenes.isEmpty())
            delete scenes.takeLast();
    }
    while (!windows.isEmpty())
        delete windows.takeLast();
    while (!documents.isEmpty())
        delete documents.takeLast();
}

Master::Status Master::readGuiConfig(const QString &filename)
{
    // Read file into JSON document
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << tr("Could not read GUI config:") << filename;
        return ReadConfigFailed;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || doc.isNull() || doc.isEmpty() || !doc.isArray())
    {
        qCritical() << tr("GUI config file is empty or parsing failed:") << error.errorString();
        qInfo() << "Note that the GUI config file must represent a single JSON array.";
        return ParseConfigFailed;
    }

    /// Map "presentation", "notes", ... to PdfDocument pointers.
    /// This is needed to interpret GUI config.
    QMap<QString, PdfMaster*> known_files;

    const QJsonArray array = doc.array();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
    for (auto it = array.begin(); it != array.end(); ++it)
#endif
    {
        if (it->type() != QJsonValue::Object)
        {
            qCritical() << tr("Ignoring invalid entry in GUI config.") << *it;
            continue;
        }
        QJsonObject obj = it->toObject();
        // Start recursive creation of widgets.
        QWidget *const widget = createWidget(obj, nullptr, known_files);
        if (!widget)
            continue;
        // Root widgets should get their own QMainWindow.
        QMainWindow *const window = new QMainWindow();
        windows.append(window);
        window->setCentralWidget(widget); // window takes ownership of widget
        if (obj.contains("title"))
            window->setWindowTitle(obj.value("title").toString());
        else
            window->setWindowTitle("BeamerPresenter");
        // Check whether this window should alawys be shown on top. This is the
        // default if the window only contains a tool selector.
        if (obj.value("always show").toBool(obj.value("type") == "tool selector"))
            // This window should always be on top.
            window->setWindowFlags(Qt::Tool|Qt::WindowStaysOnTopHint);
    }

    if (documents.isEmpty())
        return NoPDFLoaded;
    if (windows.isEmpty())
        return NoWindowsCreated;

    writable_preferences()->document = documents.first()->getDocument();
    const auto &scenes = documents.first()->getScenes();
    if (scenes.empty())
        return NoScenesCreated;
    connect(scenes.first(), &SlideScene::finishTransition, this, &Master::postNavigation);
    // Focus a scene view by default. This makes keyboard shortcuts available.
    scenes.first()->views().first()->setFocus();
    return Success;
}

QWidget* Master::createWidget(QJsonObject &object, QWidget *parent, QMap<QString, PdfMaster*> &known_files)
{
    QWidget *widget = nullptr;
    const GuiWidget type = string_to_widget_type(object.value("type").toString());
    switch (type)
    {
    case VBoxWidgetType:
    case HBoxWidgetType:
    {
        auto cwidget = new ContainerWidget(type == VBoxWidgetType ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight, parent);
        fillContainerWidget(cwidget, cwidget, object, known_files);
        widget = cwidget;
        break;
    }
    case StackedWidgetType:
    {
        auto stackwidget = new StackedWidget(parent);
        fillContainerWidget(stackwidget, stackwidget, object, known_files);
        widget = stackwidget;
        break;
    }
    case TabbedWidgetType:
    {
        auto tabwidget = new TabWidget(parent);
        widget = tabwidget;
        const QString orientation = object.value("orientation").toString().toLower();
        if (orientation == "south")
            tabwidget->setTabPosition(QTabWidget::South);
        else if (orientation == "west")
            tabwidget->setTabPosition(QTabWidget::West);
        else if (orientation == "east")
            tabwidget->setTabPosition(QTabWidget::East);
        else
            tabwidget->setTabPosition(QTabWidget::North);
        fillContainerWidget(tabwidget, tabwidget, object, known_files);
        break;
    }
    case SlideType:
    {
        auto pdf = openFile(object.value("file").toString(), known_files);
        if (pdf)
            widget = createSlide(object, pdf, parent);
        break;
    }
    case OverviewType:
    {
        PdfDocument *document = nullptr;
        const QString file = object.value("file").toString();
        if (!file.isEmpty())
        {
            auto pdf = openFile(file, known_files);
            if (pdf)
                document = pdf->getDocument();
        }
        auto twidget = new ThumbnailWidget(document, parent);
        twidget->widget()->installEventFilter(this);
        widget = twidget;
        if (object.contains("columns"))
            twidget->setColumns(object.value("columns").toInt(4));
        if (object.value("overlays").toString() == "skip")
            twidget->flags() |= ThumbnailWidget::SkipOverlays;
        connect(this, &Master::sendAction, twidget, &ThumbnailWidget::handleAction, Qt::QueuedConnection);
        connect(this, &Master::navigationSignal, twidget, &ThumbnailWidget::focusPage, Qt::QueuedConnection);
        break;
    }
    case TOCType:
        widget = new TOCwidget(parent);
        break;
    case NotesType:
    {
        auto nwidget = new NotesWidget(object.value("identifier").toString() == "number", parent);
        widget = nwidget;
        connect(this, &Master::navigationSignal, nwidget, &NotesWidget::pageChanged, Qt::QueuedConnection);
        connect(this, &Master::writeNotes, nwidget, &NotesWidget::writeNotes, Qt::DirectConnection);
        connect(this, &Master::readNotes, nwidget, &NotesWidget::readNotes, Qt::DirectConnection);
        connect(nwidget, &NotesWidget::saveDrawings, this, &Master::saveDrawings);
        connect(nwidget, &NotesWidget::loadDrawings, this, &Master::loadDrawings);
        connect(nwidget, &NotesWidget::newUnsavedChanges, this, [&](void){documents.first()->flags() |= PdfMaster::UnsavedNotes;});
        nwidget->zoomIn(object.value("zoom").toInt(10));
        if (object.contains("file"))
            nwidget->loadNotes(object.value("file").toString());
        else if (!documents.isEmpty())
            documents.first()->reloadXoppProperties();
        break;
    }
    case ToolSelectorType:
    {
        auto toolwidget = new ToolSelectorWidget(parent);
        connect(this, &Master::sendActionStatus, toolwidget, &ToolSelectorWidget::sendStatus);
        toolwidget->addButtons(object.value("buttons").toArray());
        connect(toolwidget, &ToolSelectorWidget::sendTool, this, &Master::setTool, Qt::QueuedConnection);
        connect(toolwidget, &ToolSelectorWidget::sendColor, this, &Master::sendColor);
        connect(toolwidget, &ToolSelectorWidget::sendWidth, this, &Master::sendWidth);
        connect(toolwidget, &ToolSelectorWidget::updatedTool, this, &Master::sendNewToolSoft);
        widget = toolwidget;
        break;
    }
    case ToolWidgetType:
    {
        const QBoxLayout::Direction direction = object.value("orientation").toString("horizontal") == "horizontal" ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom;
        ToolWidget *toolwidget = new ToolWidget(parent, direction);
        if (object.contains("mouse devices"))
        {
            QList<int> devices;
            int dev;
            for (const auto &dev_obj : object.value("mouse devices").toArray())
            {
                dev = string_to_input_device.value(dev_obj.toString().toStdString());
                if (dev != 0 && (dev & Tool::AnyNormalDevice) != Tool::AnyNormalDevice)
                    devices.append(dev);
            }
            if (!devices.isEmpty())
                toolwidget->setMouseDevices(devices);
        }
        if (object.contains("tablet devices"))
        {
            QList<int> devices;
            int dev;
            for (const auto &dev_obj : object.value("tablet devices").toArray())
            {
                dev = string_to_input_device.value(dev_obj.toString().toStdString());
                if (dev != 0 && (dev & Tool::AnyNormalDevice) != Tool::AnyNormalDevice)
                    devices.append(dev);
            }
            if (!devices.isEmpty())
                toolwidget->setTabletDevices(devices);
        }
        toolwidget->initialize();
        widget = toolwidget;
        break;
    }
    case SettingsType:
        widget = new SettingsWidget(parent);
        break;
    case SearchType:
    {
        widget = new SearchWidget(parent);
        connect(
            static_cast<SearchWidget*>(widget),
            &SearchWidget::searchPdf,
            this,
            [&](const QString &text, const int page, const bool forward) {documents.first()->search(text, page, forward);}
            );
        break;
    }
    case ClockType:
    {
        widget = new ClockWidget(parent, object.value("touch input").toBool(true));
        // This signal could also be connected directly to Master::sendAction,
        // but maybe the clock should be able to send different actions. Since
        // the clock rarely sends actions, this little overhead is unproblematic.
        connect(static_cast<ClockWidget*>(widget), &ClockWidget::sendAction, this, &Master::handleAction, Qt::QueuedConnection);
        break;
    }
    case AnalogClockType:
    {
        AnalogClockWidget *clock_widget = new AnalogClockWidget(parent);
        // This signal could also be connected directly to Master::sendAction,
        // but maybe the clock should be able to send different actions. Since
        // the clock rarely sends actions, this little overhead is unproblematic.
        clock_widget->readConfig(object);
        connect(clock_widget, &AnalogClockWidget::sendAction, this, &Master::handleAction, Qt::QueuedConnection);
        widget = clock_widget;
        break;
    }
    case TimerType:
    {
        TimerWidget *twidget = new TimerWidget(parent);
        connect(twidget, &TimerWidget::updateStatus, this, &Master::sendActionStatus);
        widget = twidget;
        connect(this, &Master::sendAction, twidget, &TimerWidget::handleAction, Qt::QueuedConnection);
        connect(twidget, &TimerWidget::setTimeForPage, this, &Master::setTimeForPage);
        connect(twidget, &TimerWidget::getTimeForPage, this, &Master::getTimeForPage);
        connect(this, &Master::setTotalTime, twidget, &TimerWidget::setTotalTime);
        connect(this, &Master::navigationSignal, twidget, &TimerWidget::updatePage, Qt::QueuedConnection);
        if (object.contains("colormap"))
        {
            const QJsonObject cmap_obj = object.value("colormap").toObject();
            QMap<qint32, QRgb> colormap;
            qint32 time;
            QColor color;
            bool ok;
            for (auto it = cmap_obj.constBegin(); it != cmap_obj.constEnd(); ++it)
            {
                time = it.key().toInt(&ok);
                color = QColor(it->toString());
                if (ok && color.isValid())
                    colormap[1000*time] = color.rgb();
            }
            if (!colormap.isEmpty())
                twidget->setColorMap(colormap);
        }
        if (!object.value("require confirmation").toBool(true))
            twidget->flags() |= TimerWidget::SetTimeWithoutConfirmation;
        if (object.value("confirmation default").toBool(false))
            twidget->flags() |= TimerWidget::SetTimerConfirmationDefault;
        break;
    }
    case SlideNumberType:
        widget = new SlideNumberWidget(parent);
        connect(static_cast<SlideNumberWidget*>(widget), &SlideNumberWidget::navigationSignal, this, &Master::navigateToPage);
        connect(this, &Master::navigationSignal, static_cast<SlideNumberWidget*>(widget), &SlideNumberWidget::updateText, Qt::QueuedConnection);
        break;
    case SlideLabelType:
        widget = new SlideLabelWidget(parent);
        connect(static_cast<SlideLabelWidget*>(widget), &SlideLabelWidget::navigationSignal, this, &Master::navigateToPage);
        connect(this, &Master::navigationSignal, static_cast<SlideLabelWidget*>(widget), &SlideLabelWidget::updateText, Qt::QueuedConnection);
        break;
    case GuiWidget::InvalidType:
        showErrorMessage(
                    tr("Error while reading GUI config"),
                    tr("Ignoring entry in GUI config with invalid type ") + object.value("type").toString());
        qCritical() << tr("Ignoring entry in GUI config with invalid type ") + object.value("type").toString();
        break;
    }
    if (widget)
    {
        widget->installEventFilter(this);
        // Add keyboard shortcut.
        if (object.contains("keys"))
        {
            const QKeySequence seq(object.value("keys").toString());
            if (seq.isEmpty())
                qWarning() << "Unknown key sequence in config:" << object.value("keys");
            else
                shortcuts[seq] = widget;
        }
        // Read base color from config.
        const QColor bg_color = QColor(object.value("color").toString());
        if (bg_color.isValid())
        {
            if (type == SlideType)
                static_cast<SlideView*>(widget)->setBackgroundBrush(bg_color);
            else
            {
                QPalette palette = widget->palette();
                palette.setColor(QPalette::All, QPalette::Base, bg_color);
                widget->setPalette(palette);
            }
        }
    }
    else
        qCritical() << tr("An error occured while trying to create a widget with JSON object") << object;
    return widget;
}

SlideView *Master::createSlide(QJsonObject &object, PdfMaster *pdf, QWidget *parent)
{
    if (!pdf)
        return nullptr;

    const bool is_master = object.value("master").toBool();
    if (is_master && documents.first() != pdf)
    {
        documents.removeAll(pdf);
        documents.prepend(pdf);
    }

    // Calculate the shift for scene.
    int shift = object.value("shift").toInt() & ~ShiftOverlays::AnyOverlay;
    const QString overlays = object.value("overlays").toString().toLower();
    if (overlays == "first")
        shift |= ShiftOverlays::FirstOverlay;
    else if (overlays == "last")
        shift |= ShiftOverlays::LastOverlay;

    // Get the page part.
    const QString page_part_str = object.value("page part").toString().toLower();
    PagePart page_part = FullPage;
    if (page_part_str == "left")
        page_part = LeftHalf;
    else if (page_part_str == "right")
        page_part = RightHalf;
    if (preferences()->page_part_threshold > 0.)
    {
        const QSizeF reference = pdf->getPageSize(0);
        if (reference.width() < preferences()->page_part_threshold * reference.height())
            page_part = FullPage;
    }
    if (is_master)
        writable_preferences()->default_page_part = page_part;

    SlideScene *scene {nullptr};
    // Check whether we need a new SlideScene.
    for (auto &sceneit : pdf->getScenes())
        if (sceneit->getShift() == shift && sceneit->pagePart() == page_part)
        {
            scene = sceneit;
            break;
        }

    // Create new slide scene if necessary.
    if (scene == nullptr)
    {
        scene = new SlideScene(pdf, page_part, parent);
        if (shift)
            scene->setPageShift(shift);
        if (shift == 0 || is_master)
        {
            connect(scene, &SlideScene::finishTransition, this, &Master::postNavigation);
            pdf->getScenes().prepend(scene);
        }
        else
            pdf->getScenes().append(scene);
        connect(scene, &SlideScene::newUnsavedDrawings, pdf, &PdfMaster::newUnsavedDrawings);
        connect(scene, &SlideScene::navigationSignal, this, &Master::navigateToPage, Qt::QueuedConnection);
        connect(scene, &SlideScene::sendAction, this, &Master::handleAction, Qt::QueuedConnection);
        connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
        connect(this, &Master::sendNewToolScene, scene, &SlideScene::toolChanged);
        connect(this, &Master::sendColor, scene, &SlideScene::colorChanged);
        connect(this, &Master::sendWidth, scene, &SlideScene::widthChanged);
        connect(this, &Master::postRendering, scene, &SlideScene::postRendering, Qt::QueuedConnection);
        connect(this, &Master::prepareNavigationSignal, scene, &SlideScene::prepareNavigationEvent);
        connect(pdf, &PdfMaster::updateSearch, scene, &SlideScene::updateSearchResults);
        connect(preferences(), &Preferences::stopDrawing, scene, &SlideScene::stopDrawing);
    }
    else if (is_master)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
        pdf->getScenes().swapItemsAt(pdf->getScenes().indexOf(scene), 0);
#else
        QList<SlideScene*> &list = pdf->getScenes();
        SlideScene *const tmp_scene = list.first();
        const int idx = list.indexOf(scene);
        list[0] = list[idx];
        list[idx] = tmp_scene;
#endif
    }

    // Get or create cache object.
    const PixCache *pixcache = getPixcache(
        pdf->getDocument(),
        page_part,
        object.value("cache hash").toInt(-1),
        object.value("threads").toInt(1));

    // Create slide view.
    SlideView *slide = new SlideView(scene, pixcache, parent);
    // flags for slide view
    if (object.value("media controls").toBool(false))
        slide->flags() |= SlideView::MediaControls;
    if (!object.value("show tools").toBool(true))
        slide->flags() &= ~SlideView::ShowPointingTools;
    // flags for slide scene
    if (!object.value("autoplay").toBool(true))
        scene->flags() &= ~(SlideScene::AutoplayVideo|SlideScene::AutoplaySounds);
    if (!object.value("media").toBool(true))
        scene->flags() &= ~(SlideScene::LoadMedia | SlideScene::CacheVideos | SlideScene::AutoplayVideo | SlideScene::AutoplaySounds);
    if (!object.value("transitions").toBool(true))
        scene->flags() &= ~SlideScene::ShowTransitions;
    if (!object.value("cache videos").toBool(true))
        scene->flags() &= ~SlideScene::CacheVideos;
    if (!object.value("draw").toBool(true))
        scene->flags() &= ~SlideScene::ShowDrawings;
    // Mute slides by default, except if they are marked as master.
    if (!object.value("mute").toBool(!object.value("master").toBool(false)))
        scene->flags() &= ~SlideScene::MuteSlide;
    connect(slide, &SlideView::sendAction, this, &Master::handleAction, Qt::QueuedConnection);
    connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged, Qt::DirectConnection);
    return slide;
}

PdfMaster *Master::openFile(QString name, QMap<QString, PdfMaster*> &known_files)
{
    // Check if name is known.
    if (known_files.contains(name))
        return known_files.value(name);
    // Check if name is an alias.
    if (name.isEmpty())
        name = "presentation";
    QString file = preferences()->file_alias.value(name);
    if (file.isEmpty())
        file = name;
    // Check if the alias is known.
    else if (known_files.contains(file))
        return known_files.value(file);
    // Check if file is a valid path.
    QFileInfo fileinfo(file);
    if (!fileinfo.isFile())
        // Ask the user to open a file.
        fileinfo = QFileInfo(QFileDialog::getOpenFileName(
                                 nullptr,
                                 tr("Open file") + " \"" + name + "\"",
                                 "",
                                 tr("Documents (*.pdf);;BeamerPresenter/Xournal++ files (*.bpr *.xoj *.xopp *.xml);;All files (*)")
                             ));
    if (!fileinfo.isFile())
    {
        // File does not exist, mark given aliases as invalid.
        qCritical() << tr("No valid file given");
        known_files[name] = nullptr;
        if (file != name)
            known_files[file] = nullptr;
        known_files[fileinfo.absoluteFilePath()] = nullptr;
        return nullptr;
    }
    // File exists, create a new PdfMaster object.
    const QString abs_path = fileinfo.absoluteFilePath();
    // First create an empty PdfMaster. Don't do anything with it,
    // it's not initialized yet. Only connect it to signals, some of
    // which may already be required in initialization.
    auto pdf = new PdfMaster();

    connect(this, &Master::sendAction, pdf, &PdfMaster::receiveAction);
    connect(this, &Master::navigationSignal, pdf, &PdfMaster::distributeNavigationEvents, Qt::QueuedConnection);
    connect(this, &Master::setTimeForPage, pdf, &PdfMaster::setTimeForPage);
    connect(this, &Master::getTimeForPage, pdf, &PdfMaster::getTimeForPage);
    connect(this, &Master::saveDrawings, pdf, &PdfMaster::saveXopp);
    connect(this, &Master::loadDrawings, pdf, &PdfMaster::loadXopp);
    connect(pdf, &PdfMaster::writeNotes, this, &Master::writeNotes, Qt::DirectConnection);
    connect(pdf, &PdfMaster::readNotes, this, &Master::readNotes, Qt::DirectConnection);
    connect(pdf, &PdfMaster::setTotalTime, this, &Master::setTotalTime);
    connect(pdf, &PdfMaster::navigationSignal, this, &Master::navigateToPage, Qt::DirectConnection);

    // Initialize document, try to laod PDF
    // TODO: should this be done at this point?
    pdf->initialize(abs_path);
    if (pdf->getDocument() == nullptr)
    {
        delete pdf;
        qCritical() << tr("Failed to load PDF document. This will result in errors!");
        known_files[name] = nullptr;
        known_files[abs_path] = nullptr;
        if (file != name)
            known_files[file] = nullptr;
        return nullptr;
    }

    // Adjust number of pages in preferences
    if (preferences()->number_of_pages && preferences()->number_of_pages != pdf->numberOfPages())
    {
        showErrorMessage(
                    tr("Error while loading PDF file"),
                    tr("Loaded PDF files with different numbers of pages. You should expect errors."));
        qCritical() << tr("Loaded PDF files with different numbers of pages.");
        writable_preferences()->number_of_pages = std::max(preferences()->number_of_pages, pdf->numberOfPages());
    }
    else
        writable_preferences()->number_of_pages = pdf->numberOfPages();

    documents.append(pdf);

    known_files[name] = pdf;
    known_files[abs_path] = pdf;
    if (file != name)
        known_files[file] = pdf;
    known_files[pdf->getFilename()] = pdf;
    return pdf;
}

const PixCache *Master::getPixcache(PdfDocument *doc, const PagePart page_part, int cache_hash, const int threads)
{
    if (cache_hash == -1)
        // -1 is the "default hash" and indicates that a new object has to
        // be created.
        // Set hash to -2 or smaller.
        cache_hash = (caches.isEmpty() || caches.firstKey() >= 0) ? -2 : caches.firstKey() - 1;
    else
    {
        // Check if a PixCache object with the given hash already exists.
        const PixCache *pixcache = caches.value(cache_hash, nullptr);
        if (pixcache)
            return pixcache;
    }

    // Create a new PixCache object an store it in caches.
    // Create the PixCache object.
    PixCache* pixcache = new PixCache(doc, threads, page_part);
    // Keep the new pixcache in caches.
    caches[cache_hash] = pixcache;
    // Set maximum number of pages in cache from settings.
    pixcache->setMaxNumber(preferences()->max_cache_pages);
    // Move the PixCache object to an own thread.
    pixcache->moveToThread(new QThread(pixcache));
    // Make sure that pixcache is initialized when the thread is started.
    connect(pixcache->thread(), &QThread::started, pixcache, &PixCache::init, Qt::QueuedConnection);
    connect(this, &Master::navigationSignal, pixcache, &PixCache::pageNumberChanged, Qt::QueuedConnection);
    connect(this, &Master::sendScaledMemory, pixcache, &PixCache::setScaledMemory, Qt::QueuedConnection);
    connect(this, &Master::clearCache, pixcache, &PixCache::clear, Qt::QueuedConnection);
    // Start the thread.
    pixcache->thread()->start();
    return pixcache;
}

void Master::fillContainerWidget(ContainerBaseClass *parent, QWidget *parent_widget, const QJsonObject &parent_obj, QMap<QString, PdfMaster*> &known_files)
{
    const QJsonArray array = parent_obj.value("children").toArray();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
    for (auto it = array.begin(); it != array.end(); ++it)
#endif
    {
        if (it->type() != QJsonValue::Type::Object)
        {
            qCritical() << tr("Ignoring invalid entry in GUI config.") << *it;
            continue;
        }
        QJsonObject obj = it->toObject();
        // Create child widgets recursively
        QWidget* const newwidget = createWidget(obj, parent_widget, known_files);
        QString title = obj.value("title").toString();
        if (title.isEmpty())
            title = obj.value("type").toString();
        parent->addWidgetCommon(newwidget, title);
    }
}

void Master::showAll() const
{
    for (const auto widget : qAsConst(windows))
    {
        widget->setGeometry(0, 0, 400, 300);
        widget->show();
    }
}

bool Master::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
        return QObject::eventFilter(obj, event);
    event->ignore();
    obj->event(event);
    if (event->isAccepted())
        return true;
    const auto *kevent = static_cast<QKeyEvent*>(event);
#if (QT_VERSION_MAJOR >= 6)
    const QKeySequence key_code(kevent->keyCombination());
#else
    const quint32 key_code = kevent->key() | (kevent->modifiers() & ~Qt::KeypadModifier);
#endif
    // Search shortcuts for given key sequence.
    {
        QWidget* widget = shortcuts.value(key_code);
        debug_msg(DebugKeyInput, "Key action:" << widget << kevent << (kevent->key() | (kevent->modifiers() & ~Qt::KeypadModifier)));
        if (widget)
        {
            widget->show();
            widget->setFocus();
            QStackedWidget *stackwidget = dynamic_cast<QStackedWidget*>(widget->parentWidget());
            debug_msg(DebugKeyInput, widget << stackwidget << widget->parentWidget());
            if (stackwidget)
            {
                QTabWidget *tabwidget = dynamic_cast<QTabWidget*>(stackwidget->parentWidget());
                if (tabwidget)
                    tabwidget->setCurrentWidget(widget);
                else
                    stackwidget->setCurrentWidget(widget);
            }
        }
    }
    // Search actions in preferences for given key sequence.
    {
        for (Action action : static_cast<const QList<Action>>(preferences()->key_actions.values(key_code)))
        {
            debug_msg(DebugKeyInput, "Global key action:" << action);
            handleAction(action);
        }
    }
    // Search tools in preferences for given key sequence.
    for (const auto tool : static_cast<const QList<Tool*>>(preferences()->key_tools.values(key_code)))
        if (tool && tool->device())
            setTool(tool->copy());
    event->accept();
    return true;
}

void Master::handleAction(const Action action)
{
    switch (action)
    {
    case InvalidAction:
    case NoAction:
        break;
    case Update:
        navigateToPage(preferences()->page);
        break;
    case NextPage:
        navigateToPage(preferences()->page + 1);
        break;
    case PreviousPage:
        navigateToPage(preferences()->page - 1);
        break;
    case NextSkippingOverlays:
        navigateToPage(documents.first()->overlaysShifted(preferences()->page, 1 | FirstOverlay));
        break;
    case PreviousSkippingOverlays:
        navigateToPage(documents.first()->overlaysShifted(preferences()->page, -1 & ~FirstOverlay));
        break;
    case FirstPage:
        navigateToPage(0);
        break;
    case LastPage:
        navigateToPage(preferences()->number_of_pages - 1);
        break;
    case FullScreen:
        for (const auto win : qAsConst(windows))
        {
            if (win->isActiveWindow())
            {
                if (win->isFullScreen())
                    win->showNormal();
                else
                    win->showFullScreen();
                break;
            }
        }
        break;
    case SaveDrawings:
    {
        if (documents.isEmpty())
            break;
        PdfMaster *doc = documents.first();
        if (!doc)
            break;
        QString filename = doc->drawingsPath();
        if (filename.isEmpty())
            filename = getSaveFileName();
        doc->saveXopp(filename);
        break;
    }
    case SaveDrawingsAs:
    {
        if (documents.isEmpty())
            break;
        const QString filename = getSaveFileName();
        if (!filename.isEmpty() && documents.first())
            documents.first()->saveXopp(filename);
        break;
    }
    case LoadDrawings:
    {
        if (documents.isEmpty())
            break;
        const QString filename = getOpenFileName();
        if (!filename.isEmpty())
        {
            PdfMaster *doc = documents.first();
            if (!doc)
                break;
            doc->clearAllDrawings();
            doc->loadXopp(filename);
            doc->flags() &= ~PdfMaster::UnsavedDrawings;
        }
        navigateToPage(preferences()->page);
        break;
    }
    case LoadDrawingsNoClear:
    {
        if (documents.isEmpty())
            break;
        const QString filename = getOpenFileName();
        if (filename.isEmpty() || documents.first() == NULL)
            break;
        documents.first()->loadXopp(filename);
        navigateToPage(preferences()->page);
        break;
    }
    case ReloadFiles:
    {
        // TODO: problems with slide labels, navigation, and videos after reloading files
        bool changed = false;
        for (const auto doc : qAsConst(documents))
            changed |= doc->loadDocument();
        if (changed)
        {
            writable_preferences()->number_of_pages = documents.first()->numberOfPages();
            distributeMemory();
            emit clearCache();
            emit sendAction(PdfFilesChanged);
            navigateToPage(preferences()->page);
        }
        break;
    }
    case ResizeViews:
        distributeMemory();
        break;
    case Quit:
        if (!askCloseConfirmation())
            break;
        [[clang::fallthrough]];
    case QuitNoConfirmation:
        for (const auto window : qAsConst(windows))
            window->close();
        break;
    case Mute:
        debug_msg(DebugMedia, "muting application");
        writable_preferences()->global_flags |= Preferences::MuteApplication;
        emit sendAction(action);
        break;
    case Unmute:
        debug_msg(DebugMedia, "unmuting application");
        writable_preferences()->global_flags &= ~Preferences::MuteApplication;
        emit sendAction(action);
        break;
    default:
        emit sendAction(action);
    }
}

bool Master::askCloseConfirmation() const noexcept
{
    if (documents.isEmpty())
        return true;
    PdfMaster *doc = documents.first();
    if (doc && (
            (doc->flags() & (PdfMaster::UnsavedNotes | PdfMaster::UnsavedTimes))
            || ((doc->flags() & PdfMaster::UnsavedDrawings) && doc->hasDrawings())
                ))
    {
        debug_msg(DebugWidgets, "Asking for close confirmation:" << doc->flags());
        switch (QMessageBox::question(
                    nullptr,
                    Master::tr("Unsaved changes"),
                    Master::tr("The document may contain unsaved changes. Quit anyway?"),
                    QMessageBox::Close | QMessageBox::Save | QMessageBox::Cancel,
                    QMessageBox::Save
                ))
        {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
        {
            QString filename = doc->drawingsPath();
            if (filename.isEmpty())
                filename = getSaveFileName();
            doc->saveXopp(filename);
            break;
        }
        default:
            break;
        }
    }
    return true;
}

void Master::leavePage(const int page) const
{
    writable_preferences()->previous_page = page;
    bool flexible_page_numbers = false;
    for (const auto doc : qAsConst(documents))
    {
        doc->clearHistory(page, preferences()->history_length_hidden_slides);
        doc->clearHistory(page | PagePart::LeftHalf, preferences()->history_length_hidden_slides);
        doc->clearHistory(page | PagePart::RightHalf, preferences()->history_length_hidden_slides);
        if (doc->flexiblePageSizes())
            flexible_page_numbers = true;
    }
    if (flexible_page_numbers)
    {
        QLayout *layout;
        for (const auto window : qAsConst(windows))
        {
            window->updateGeometry();
            layout = window->layout();
            if (layout)
                layout->invalidate();
        }
    }
}

void Master::distributeMemory()
{
    if (preferences()->max_memory < 0)
        return;
    float scale = 0.;
    for (const auto cache : qAsConst(caches))
        scale += cache->getPixels();
    if (scale <= 0)
        return;
    scale = preferences()->max_memory / scale;
    debug_msg(DebugCache, "Distributing memory. scale =" << scale << ", max. memory =" << preferences()->max_memory);
    emit sendScaledMemory(scale);
}

qint64 Master::getTotalCache() const
{
    qint64 cache = 0;
    for (const auto px : qAsConst(caches))
        cache += px->getUsedMemory();
    return cache;
}

void Master::navigateToPage(const int page)
{
    if (page < 0 || page >= preferences()->number_of_pages)
        return;
    if (cacheVideoTimer_id != -1)
    {
        killTimer(cacheVideoTimer_id);
        cacheVideoTimer_id = -1;
    }
    if (slideDurationTimer_id != -1)
    {
        killTimer(slideDurationTimer_id);
        slideDurationTimer_id = -1;
    }
    leavePage(preferences()->page);
    emit prepareNavigationSignal(page);
    for (const auto window : windows)
        window->updateGeometry();
    // Get duration of the slide: But only take a nontrivial value if
    // the new page is (old page + 1).
    writable_preferences()->page = page;
    emit navigationSignal(page);
}

void Master::postNavigation() noexcept
{
    if (slideDurationTimer_id != -1 || cacheVideoTimer_id != -1)
        return;
    const int page = preferences()->page;
    const qreal duration =
            preferences()->global_flags & Preferences::AutoSlideChanges && page == preferences()->previous_page + 1
            ? documents.first()->getDocument()->duration(preferences()->page)
            : -1.;
    if (duration == 0.)
        slideDurationTimer_id = startTimer(preferences()->slide_duration_animation);
    else if (duration > 0.)
        slideDurationTimer_id = startTimer(1000*duration);
    if (duration < 0. || duration > 0.5)
        cacheVideoTimer_id = startTimer(200);
}

void Master::showErrorMessage(const QString &title, const QString &text) const
{
    QMessageBox::critical(
                windows.isEmpty() ? NULL : windows.first(),
                title,
                text);
}


void Master::setTool(Tool *tool) const noexcept
{
    if (!tool || !tool->device())
    {
        delete tool;
        return;
    }
    debug_msg(DebugDrawing|DebugKeyInput, "Set tool" << tool->tool() << tool->device());
    writable_preferences()->setCurrentTool(tool);
    emit sendNewToolScene(tool);
    emit sendNewToolSoft(tool);

#ifdef QT_DEBUG
    if ((preferences()->debug_level & DebugVerbose) && preferences()->debug_level & DebugDrawing)
    {
        const auto &current_tools = preferences()->current_tools;
        const auto end = current_tools.cend();
        for (auto tool = current_tools.cbegin(); tool != end; ++tool)
            if (*tool)
                qDebug() << "tool:" << (*tool)->device() << (*tool)->tool() << *tool;
    }
#endif
}

QString Master::getOpenFileName()
{
    return QFileDialog::getOpenFileName(
                NULL,
                tr("Load drawings"),
                "",
                tr("BeamerPresenter/Xournal++ files (*.bpr *.xoj *.xopp *.xml);;All files (*)")
            );
}

QString Master::getSaveFileName()
{
    return QFileDialog::getSaveFileName(
                NULL,
                tr("Save drawings"),
                "",
                tr("BeamerPresenter/Xournal++ files (*.bpr *.xopp);;All files (*)")
            );
}

void Master::timerEvent(QTimerEvent *event)
{
    debug_msg(DebugPageChange, "timer event" << event->timerId() << cacheVideoTimer_id << slideDurationTimer_id);
    killTimer(event->timerId());
    if (event->timerId() == cacheVideoTimer_id)
    {
        cacheVideoTimer_id = -1;
        emit postRendering();
    }
    else if (event->timerId() == slideDurationTimer_id)
    {
        slideDurationTimer_id = -1;
        nextSlide();
    }
}
