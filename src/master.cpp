#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"
#include "src/slideview.h"
#include "src/drawing/drawtool.h"
#include "src/drawing/pointingtool.h"
#include "src/gui/flexlayout.h"
#include "src/gui/clockwidget.h"
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
#include "src/rendering/pixcache.h"
#include "src/names.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QThread>

Master::Master() :
    cacheVideoTimer(new QTimer(this)),
    slideDurationTimer(new QTimer(this))
{
    cacheVideoTimer->setSingleShot(true);
    cacheVideoTimer->setInterval(200);
    slideDurationTimer->setSingleShot(true);
    connect(slideDurationTimer, &QTimer::timeout, this, &Master::nextSlide);
}

Master::~Master()
{
    delete cacheVideoTimer;
    delete slideDurationTimer;
    for (const auto cache : qAsConst(caches))
        cache->thread()->quit();
    for (const auto cache : qAsConst(caches))
    {
        cache->thread()->wait(10000);
        delete cache;
    }
    caches.clear();
    for (const auto doc : qAsConst(documents))
    {
        qDeleteAll(doc->getScenes());
        doc->getScenes().clear();
    }
    qDeleteAll(windows);
    qDeleteAll(documents);
}

unsigned char Master::readGuiConfig(const QString &filename)
{
    // Read file into JSON document
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not read GUI config:" << filename;
        return 1;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || doc.isNull() || doc.isEmpty() || !doc.isArray())
    {
        qCritical() << "GUI config file is empty or parsing failed:" << error.errorString();
        qInfo() << "Note that the GUI config file must represent a single JSON array.";
        return 2;
    }
    const QJsonArray array = doc.array();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
    for (auto it = array.begin(); it != array.end(); ++it)
#endif
    {
        if (it->type() != QJsonValue::Object)
        {
            qCritical() << "Ignoring invariant entry in GUI config.";
            continue;
        }
        QJsonObject obj = it->toObject();
        // Start recursive creation of widgets.
        QWidget *const widget = createWidget(obj, NULL);
        if (!widget)
            continue;
        // Root widgets should get their own QMainWindow.
        QMainWindow *const window = new QMainWindow();
        windows.append(window);
        window->setCentralWidget(widget); // window takes ownership of widget
        window->setWindowTitle("BeamerPresenter");
        // Check whether this window should alawys be shown on top. This is the
        // default if the window only contains a tool selector.
        if (obj.value("always show").toBool(obj.value("type") == "tool selector"))
            // This window should always be on top.
            window->setWindowFlags(Qt::Tool|Qt::WindowStaysOnTopHint);
    }

    if (documents.isEmpty())
        return 4;

    writable_preferences()->document = documents.first()->getDocument();

    // Return true (success) if at least one window and one document were created.
    if (windows.isEmpty())
        return 3;
    return 0;
}

QWidget* Master::createWidget(QJsonObject &object, QWidget *parent)
{
    if (!object.contains("type"))
    {
        if (object.contains("children"))
            object.insert("type", "container");
        else if (object.contains("file"))
            object.insert("type", "slide");
        else
        {
            qCritical() << "Ignoring entry in GUI config without type." << object;
            return NULL;
        }
    }
    QWidget *widget = NULL;
    const GuiWidget type = string_to_widget_type.value(object.value("type").toString().toLower(), GuiWidget::InvalidType);
    switch (type)
    {
    case VBoxWidgetType:
    case HBoxWidgetType:
    {
        widget = new ContainerWidget(parent);
        FlexLayout* layout = new FlexLayout(type == VBoxWidgetType ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
        layout->setContentsMargins(0, 0, 0, 0);

        const QJsonArray array = object.value("children").toArray();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
        for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
        for (auto it = array.begin(); it != array.end(); ++it)
#endif
        {
            if (it->type() != QJsonValue::Type::Object)
            {
                qCritical() << "Ignoring invalid entry in GUI config.";
                continue;
            }
            QJsonObject obj = it->toObject();
            // Create child widgets recursively
            QWidget* const newwidget = createWidget(obj, widget);
            if (newwidget)
                layout->addWidget(newwidget);
        }
        widget->setLayout(layout);
        break;
    }
    case StackedWidgetType:
    {
        StackedWidget *stackwidget = new StackedWidget(parent);
        const QJsonArray array = object.value("children").toArray();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
        for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
        for (auto it = array.begin(); it != array.end(); ++it)
#endif
        {
            if (it->type() != QJsonValue::Type::Object)
            {
                qCritical() << "Ignoring invalid entry in GUI config.";
                continue;
            }
            QJsonObject obj = it->toObject();
            // Create child widgets recursively
            QWidget* const newwidget = createWidget(obj, widget);
            if (newwidget)
            {
                stackwidget->addWidget(newwidget);
                if (!obj.contains("keys"))
                    qWarning() << "Widget in stack layout without key shortcuts is inaccessible.";
            }
        }
        widget = stackwidget;
        break;
    }
    case TabbedWidgetType:
    {
        TabWidget *tabwidget = new TabWidget(parent);
        tabwidget->setTabPosition(string_to_tab_widget_orientation.value(object.value("orientation").toString()));

        const QJsonArray array = object.value("children").toArray();
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
        for (auto it = array.cbegin(); it != array.cend(); ++it)
#else
        for (auto it = array.begin(); it != array.end(); ++it)
#endif
        {
            if (it->type() != QJsonValue::Type::Object)
            {
                qCritical() << "Ignoring invalid entry in GUI config.";
                continue;
            }
            QJsonObject obj = it->toObject();
            // Create child widgets recursively
            QWidget* const newwidget = createWidget(obj, widget);
            if (obj.contains("title"))
                tabwidget->addTab(newwidget, obj.value("title").toString());
            else
                tabwidget->addTab(newwidget, obj.value("type").toString());
        }
        widget = tabwidget;
        break;
    }
    case SlideType:
    {
        // Calculate the shift for scene.
        int shift = object.value("shift").toInt() & ~ShiftOverlays::AnyOverlay;
        const QString overlays = object.value("overlays").toString().toLower();
        if (overlays == "first")
            shift |= ShiftOverlays::FirstOverlay;
        else if (overlays == "last")
            shift |= ShiftOverlays::LastOverlay;

        // Find PDF file name.
        // Usually "file" will be "presentation" or "notes". These keywords
        // are mapped to filenames by preferences()->file_alias.
        QString file = object.value("file").toString();
        file = preferences()->file_alias.value(file.isEmpty() ? "presentation" : file, file);
        if (file == "//INVALID")
            break;
        QFileInfo fileinfo(file);
        if (fileinfo.exists())
            file = fileinfo.absoluteFilePath();
        else
        {
            fileinfo = QFileInfo(QFileDialog::getOpenFileName(
                                     NULL,
                                     "Open file \"" + file + "\"",
                                     "",
                                     "Documents (*.pdf);;BeamerPresenter/Xournal++ files (*.bpr *.xoj *.xopp *.xml);;All files (*)"
                                 ));
            if (!fileinfo.exists())
            {
                qCritical() << "No valid file given";
                writable_preferences()->file_alias.insert(file, "//INVALID");
                writable_preferences()->file_alias.insert(object.value("file").toString(), "//INVALID");
                break;
            }
            const QString oldfile = file;
            file = fileinfo.absoluteFilePath();
            writable_preferences()->file_alias.insert(oldfile, file);
            writable_preferences()->file_alias.insert(object.value("file").toString(), file);
        }
        const QString page_part_str = object.value("page part").toString().toLower();
        PagePart page_part = FullPage;
        if (page_part_str == "left")
            page_part = LeftHalf;
        else if (page_part_str == "right")
            page_part = RightHalf;

        // Check whether the PDF has been loaded already, load it if necessary.
        PdfMaster *doc = NULL;
        SlideScene *scene = NULL;
        for (auto docit = documents.begin(); docit != documents.end(); ++docit)
        {
            if ((*docit)->getFilename() == file || (*docit)->drawingsPath() == file)
            {
                doc = *docit;
                if (page_part != FullPage && preferences()->page_part_threshold > 0.)
                {
                    const QSizeF reference = doc->getPageSize(0);
                    if (reference.width() < preferences()->page_part_threshold * reference.height())
                        page_part = FullPage;
                }
                if (object.value("master").toBool())
                {
                    documents.erase(docit);
                    documents.prepend(doc);
                    writable_preferences()->default_page_part = page_part;
                }
                break;
            }
        }
        if (doc == NULL)
        {
            // First create an empty PdfMaster. Don't do anything with it,
            // it's not initialized yet. Only connect it to signals, some of
            // which may already be required in initialization.
            doc = new PdfMaster();
            connect(this, &Master::sendAction, doc, &PdfMaster::receiveAction);
            connect(doc, &PdfMaster::navigationSignal, this, &Master::navigateToPage, Qt::QueuedConnection);
            connect(this, &Master::navigationSignal, doc, &PdfMaster::distributeNavigationEvents, Qt::QueuedConnection);
            connect(this, &Master::setTimeForPage, doc, &PdfMaster::setTimeForPage);
            connect(this, &Master::getTimeForPage, doc, &PdfMaster::getTimeForPage);
            connect(this, &Master::saveDrawings, doc, &PdfMaster::saveXopp);
            connect(this, &Master::loadDrawings, doc, &PdfMaster::loadXopp);
            connect(doc, &PdfMaster::writeNotes, this, &Master::writeNotes, Qt::DirectConnection);
            connect(doc, &PdfMaster::readNotes, this, &Master::readNotes, Qt::DirectConnection);
            connect(doc, &PdfMaster::setTotalTime, this, &Master::setTotalTime);
            // Initialize doc with file.
            doc->initialize(file);
            if (doc->getDocument() == NULL)
            {
                qCritical() << "Failed to load PDF document. This will result in errors!";
                delete doc;
                return NULL;
            }
            if (writable_preferences()->number_of_pages && writable_preferences()->number_of_pages != doc->numberOfPages())
            {
                qWarning() << "Loaded PDF files with different numbers of pages. You should expect errors.";
                writable_preferences()->number_of_pages = std::max(writable_preferences()->number_of_pages, doc->numberOfPages());
            }
            else
                writable_preferences()->number_of_pages = doc->numberOfPages();
            if (preferences()->page_part_threshold > 0.)
            {
                const QSizeF reference = doc->getPageSize(0);
                if (reference.width() < preferences()->page_part_threshold * reference.height())
                    page_part = FullPage;
            }
            if (object.value("master").toBool())
            {
                documents.prepend(doc);
                writable_preferences()->default_page_part = page_part;
            }
            else
                documents.append(doc);
        }
        else {
            // If PDF files existed before, check whether we need a new SlideScene.
            for (auto &sceneit : doc->getScenes())
            {
                if (sceneit->identifier() == qHash(QPair<int, const void*>(shift, doc)) + page_part)
                {
                    scene = sceneit;
                    break;
                }
            }
        }

        // Create new slide scene if necessary.
        if (scene == NULL)
        {
            scene = new SlideScene(doc, page_part, parent);
            if (shift)
                scene->setPageShift(shift);
            if (shift == 0 || object.value("master").toBool(false))
            {
                connect(scene, &SlideScene::finishTransition, this, &Master::postNavigation);
                doc->getScenes().prepend(scene);
            }
            else
                doc->getScenes().append(scene);
            connect(scene, &SlideScene::newUnsavedDrawings, doc, &PdfMaster::newUnsavedDrawings);
            connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
            connect(cacheVideoTimer, &QTimer::timeout, scene, &SlideScene::postRendering, Qt::QueuedConnection);
            connect(this, &Master::prepareNavigationSignal, scene, &SlideScene::prepareNavigationEvent);
            connect(preferences(), &Preferences::stopDrawing, scene, &SlideScene::stopDrawing);
        }
        else if (object.value("master").toBool())
        {
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
            doc->getScenes().swapItemsAt(doc->getScenes().indexOf(scene), 0);
#else
            QList<SlideScene*> &list = doc->getScenes();
            SlideScene *const tmp_scene = list.first();
            const int idx = list.indexOf(scene);
            list[0] = list[idx];
            list[idx] = tmp_scene;
#endif
        }
        // TODO: read other properties from config

        // Get or create cache object.
        PixCache *pixcache = NULL;
        int cache_hash = object.value("cache hash").toInt(-1);
        if (cache_hash == -1)
        {
            // -1 is the "default hash" and indicates that a new object has to
            // be created.
            // Set hash to -2 or smaller.
            cache_hash = (caches.isEmpty() || caches.firstKey() >= 0) ? -2 : caches.firstKey() - 1;
        }
        else
        {
            // Check if a PixCache object with the given hash already exists.
            pixcache = caches.value(cache_hash, NULL);
        }
        // If necessary, create a new PixCache object an store it in caches.
        if (pixcache == NULL)
        {
            // Read number of threads from GUI config.
            const int threads = object.value("threads").toInt(1);
            // Create the PixCache object.
            pixcache = new PixCache(scene->getPdfMaster()->getDocument(), threads, page_part);
            // Move the PixCache object to an own thread.
            pixcache->moveToThread(new QThread());
            connect(pixcache, &PixCache::destroyed, pixcache->thread(), &QThread::deleteLater);
            // Make sure that pixcache is initialized when the thread is started.
            connect(pixcache->thread(), &QThread::started, pixcache, &PixCache::init, Qt::QueuedConnection);
            connect(this, &Master::navigationSignal, pixcache, &PixCache::pageNumberChanged, Qt::QueuedConnection);
            // Set maximum number of pages in cache from settings.
            pixcache->setMaxNumber(preferences()->max_cache_pages);
            // Start the thread.
            pixcache->thread()->start();
            // Keep the new pixcache in caches.
            caches[cache_hash] = pixcache;
        }

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
        if (!object.value("mute").toBool(true))
            scene->flags() &= ~SlideScene::Mute;
        connect(slide, &SlideView::sendKeyEvent, this, &Master::receiveKeyEvent);
        connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged, Qt::DirectConnection);
        widget = slide;
        break;
    }
    case OverviewType:
    {
        ThumbnailWidget *twidget = new ThumbnailWidget(parent);
        widget = twidget;
        if (object.contains("columns"))
            twidget->setColumns(object.value("columns").toInt(4));
        connect(twidget, &ThumbnailWidget::sendNavigationSignal, this, &Master::navigateToPage);
        if (object.value("overlays").toString() == "skip")
            twidget->flags() |= ThumbnailWidget::SkipOverlays;
        break;
    }
    case TOCType:
        widget = new TOCwidget(parent);
        connect(static_cast<TOCwidget*>(widget), &TOCwidget::sendNavigationSignal, this, &Master::navigateToPage);
        break;
    case NotesType:
    {
        NotesWidget *nwidget = new NotesWidget(object.value("identifier").toString() == "number", parent);
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
        ToolSelectorWidget *toolwidget = new ToolSelectorWidget(parent);
        const QJsonArray full_array = object.value("buttons").toArray();
        for (int i=0; i<full_array.size(); i++)
        {
            const QJsonArray row = full_array[i].toArray();
            for (int j=0; j<row.size(); j++)
            {
                switch (row[j].type())
                {
                case QJsonValue::String:
                    toolwidget->addActionButton(i, j, row[j].toString());
                    break;
                case QJsonValue::Array:
                    toolwidget->addActionButton(i, j, row[j].toArray());
                    break;
                case QJsonValue::Object:
                {
                    Tool *tool = createTool(row[j].toObject(), 0);
                    if (tool)
                        toolwidget->addToolButton(i, j, tool);
                    else
                        qWarning() << "Failed to create tool button" << row[j].toObject().value("tool");
                    break;
                }
                default:
                    break;
                }
            }
        }
        connect(toolwidget, &ToolSelectorWidget::sendAction, this, &Master::handleAction, Qt::QueuedConnection);
        connect(toolwidget, &ToolSelectorWidget::sendTool, this, &Master::setTool, Qt::QueuedConnection);
        widget = toolwidget;
        break;
    }
    case SettingsType:
        widget = new SettingsWidget(parent);
        break;
    case ClockType:
        widget = new ClockWidget(parent);
        // This signal could also be connected directly to Master::sendAction,
        // but maybe the clock should be able to send different actions. Since
        // the clock rarely sends actions, this little overhead is unproblematic.
        connect(static_cast<ClockWidget*>(widget), &ClockWidget::sendAction, this, &Master::handleAction);
        break;
    case TimerType:
    {
        TimerWidget *twidget = new TimerWidget(parent);
        widget = twidget;
        connect(this, &Master::sendAction, twidget, &TimerWidget::handleAction);
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
        qWarning() << "Ignoring entry in GUI config with invalid type:" << object.value("type");
    }
    if (widget)
    {
        // Add keyboard shortcut.
        if (object.contains("keys"))
        {
            const QKeySequence seq(object.value("keys").toString());
            if (seq.isEmpty())
                qWarning() << "Unknown key sequence in config:" << object.value("keys");
            else
                shortcuts[seq[0] + seq[1] + seq[2] + seq[3]] = widget;
        }
        // Read base color from config or take it from parent.
        QPalette palette = (parent ? parent : widget)->palette();
        const QColor bg_color = QColor(object.value("color").toString());
        if (bg_color.isValid())
            palette.setColor(QPalette::Base, bg_color);
        widget->setPalette(palette);
    }
    else
        qWarning() << "An error occured while trying to create a widget of type" << object.value("type");
    return widget;
}

void Master::showAll() const
{
    for (const auto widget : qAsConst(windows))
    {
        widget->setGeometry(0, 0, 400, 300);
        widget->show();
    }
}

void Master::receiveKeyEvent(const QKeyEvent* event)
{
    const quint32 key_code = event->key() | (event->modifiers() & ~Qt::KeypadModifier);
    // Search shortcuts for given key sequence.
    {
        QWidget* widget = shortcuts.value(key_code);
        debug_msg(DebugKeyInput) << "Key action:" << widget << event << (event->key() | (event->modifiers() & ~Qt::KeypadModifier));
        if (widget)
        {
            widget->show();
            QStackedWidget *stackwidget = dynamic_cast<QStackedWidget*>(widget->parentWidget());
            debug_msg(DebugKeyInput) << widget << stackwidget << widget->parentWidget();
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
            debug_msg(DebugKeyInput) << "Global key action:" << action;
            handleAction(action);
        }
    }
    // Search tools in preferences for given key sequence.
    for (const auto tool : static_cast<const QList<Tool*>>(preferences()->key_tools.values(key_code)))
    {
        if (tool && tool->device())
        {
            if (tool->tool() & Tool::AnyDrawTool)
                setTool(new DrawTool(*static_cast<const DrawTool*>(tool)));
            else if (tool->tool() & Tool::AnyPointingTool)
                setTool(new PointingTool(*static_cast<const PointingTool*>(tool)));
            else
                setTool(new Tool(*tool));
        }
    }
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
        bool changed = false;
        for (const auto doc : qAsConst(documents))
            changed |= doc->loadDocument();
        if (changed)
        {
            writable_preferences()->number_of_pages = documents.first()->numberOfPages();
            for (const auto cache : qAsConst(caches))
                cache->clear();
            navigateToPage(preferences()->page);
        }
        break;
    }
    case Quit:
        if (!documents.isEmpty())
        {
            PdfMaster *doc = documents.first();
            if (doc && doc->flags() & (PdfMaster::UnsavedDrawings | PdfMaster::UnsavedNotes | PdfMaster::UnsavedTimes))
            {
                debug_msg(DebugWidgets) << "Asking for close confirmation:" << doc->flags();
                switch (QMessageBox::question(
                            NULL,
                            "Unsaved changes",
                            "The document may contain unsaved changes. Quit anyway?",
                            QMessageBox::Close | QMessageBox::Save | QMessageBox::Cancel,
                            QMessageBox::Save
                        ))
                {
                case QMessageBox::Cancel:
                    return;
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
        }
        [[clang::fallthrough]];
    case QuitNoConfirmation:
        for (const auto window : qAsConst(windows))
            window->close();
        break;
    default:
        emit sendAction(action);
    }
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
    for (const auto cache : qAsConst(caches))
        cache->setScaledMemory(scale);
}

qint64 Master::getTotalCache() const
{
    qint64 cache = 0;
    for (const auto px : qAsConst(caches))
        cache += px->getUsedMemory();
    return cache;
}

void Master::navigateToPage(const int page) const
{
    if (page < 0 || page >= preferences()->number_of_pages)
        return;
    slideDurationTimer->stop();
    cacheVideoTimer->stop();
    leavePage(preferences()->page);
    emit prepareNavigationSignal(page);
    for (auto window : windows)
        window->updateGeometry();
    // Get duration of the slide: But only take a nontrivial value if
    // the new page is (old page + 1).
    writable_preferences()->page = page;
    emit navigationSignal(page);
}

void Master::postNavigation() const noexcept
{
    if (slideDurationTimer->isActive() || cacheVideoTimer->isActive())
        return;
    const int page = preferences()->page;
    const qreal duration =
            preferences()->global_flags & Preferences::AutoSlideChanges && page == preferences()->previous_page + 1
            ? documents.first()->getDocument()->duration(preferences()->page)
            : -1.;
    if (duration == 0.)
        slideDurationTimer->start(preferences()->slide_duration_animation);
    else if (duration > 0.)
        slideDurationTimer->start(1000*duration);
    if (duration < 0. || duration > 0.5)
        cacheVideoTimer->start();
}

void Master::setTool(Tool *tool) const noexcept
{
    if (!tool || !tool->device())
    {
        delete tool;
        return;
    }
    debug_msg(DebugDrawing|DebugKeyInput) << "Set tool" << tool->tool() << tool->device();
    int device = tool->device();
    // Delete mouse no button devices if MouseLeftButton is overwritten.
    if (tool->device() & Tool::MouseLeftButton)
        device |= Tool::MouseNoButton;
    // Delete tablet no pressure device if any tablet device is overwritten.
    if (tool->device() & (Tool::TabletCursor | Tool::TabletPen | Tool::TabletEraser))
        device |= Tool::TabletNoPressure;
    int newdevice;
    for (auto tool_it = preferences()->current_tools.cbegin(); tool_it != preferences()->current_tools.cend();)
    {
        if ((*tool_it)->device() & device)
        {
            newdevice = (*tool_it)->device() & ~device;
            if (newdevice)
                (*tool_it++)->setDevice(newdevice);
            else
            {
                delete *tool_it;
                tool_it = static_cast<QSet<Tool*>::const_iterator>(writable_preferences()->current_tools.erase(tool_it));
            }
        }
        else if (((*tool_it)->device() == Tool::MouseNoButton) && (tool->device() & Tool::MouseLeftButton))
        {
            delete *tool_it;
            tool_it = static_cast<QSet<Tool*>::const_iterator>(writable_preferences()->current_tools.erase(tool_it));
        }
        else
            ++tool_it;
    }
    writable_preferences()->current_tools.insert(tool);

#ifdef QT_DEBUG
    if ((preferences()->debug_level & DebugVerbose) && preferences()->debug_level & DebugDrawing)
        for (const auto tool : preferences()->current_tools)
            qDebug() << "tool:" << tool->device() << tool->tool() << tool;
#endif
}

QString Master::getOpenFileName()
{
    return QFileDialog::getOpenFileName(
                NULL,
                "Load drawings",
                "",
                "BeamerPresenter/Xournal++ files (*.bpr *.xoj *.xopp *.xml);;All files (*)"
            );
}

QString Master::getSaveFileName()
{
    return QFileDialog::getSaveFileName(
                NULL,
                "Save drawings",
                "",
                "BeamerPresenter/Xournal++ files (*.bpr *.xopp);;All files (*)"
            );
}
