#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"

Master::~Master()
{
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

bool Master::readGuiConfig(const QString &filename)
{
    // Read file into JSON document
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qCritical() << "Could not read GUI config:" << filename;
        return false;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError)
    {
        qCritical() << "Parsing GUI config failed:" << error.errorString();
        return false;
    }
    if (doc.isNull() || doc.isEmpty())
    {
        qCritical() << "GUI config file is empty or parsing failed";
        return false;
    }
    if (!doc.isArray())
    {
        qCritical() << "Interpreting GUI file failed: must be a JSON array";
        return false;
    }
    const QJsonArray array = doc.array();
    for (auto it = array.cbegin(); it != array.cend(); ++it)
    {
        if (it->type() != QJsonValue::Object)
        {
            qCritical() << "Ignoring invariant entry in GUI config.";
            continue;
        }
        QJsonObject obj = it->toObject();
        // Start recursive creation of widgets.
        QWidget* const widget = createWidget(obj, NULL);
        if (widget)
            windows.append(widget);
    }

    if (!documents.isEmpty())
        writable_preferences()->document = documents.first()->getDocument();

    // Return true (success) if at least one window and one document were created.
    return !windows.isEmpty() && !documents.isEmpty();
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
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        FlexLayout* layout = new FlexLayout(type == VBoxWidgetType ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
        layout->setContentsMargins(0, 0, 0, 0);

        const QJsonArray array = object.value("children").toArray();
        for (auto it = array.cbegin(); it != array.cend(); ++it)
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
        for (auto it = array.cbegin(); it != array.cend(); ++it)
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
        QTabWidget *tabwidget = new QTabWidget(parent);
        tabwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        tabwidget->setTabPosition(string_to_tab_widget_orientation.value(object.value("orientation").toString()));

        const QJsonArray array = object.value("children").toArray();
        for (auto it = array.cbegin(); it != array.cend(); ++it)
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
        if (!QFile::exists(file))
        {
            const QString newfile = QFileDialog::getOpenFileName(NULL, "File " + file, "", "Documents (*.pdf)");
            if (!QFile::exists(newfile))
            {
                qCritical() << "No valid file given";
                writable_preferences()->file_alias.insert(file, "//INVALID");
                break;
            }
            writable_preferences()->file_alias.insert(file, newfile);
            writable_preferences()->file_alias.insert(object.value("file").toString(), newfile);
            file = newfile;
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
            if ((*docit)->getFilename() == file)
            {
                doc = *docit;
                if (preferences()->page_part_threshold > 0.)
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
            doc = new PdfMaster(file);
            if (writable_preferences()->number_of_pages && writable_preferences()->number_of_pages != doc->numberOfPages())
            {
                qWarning() << "Loaded PDF files with different numbers of pages. You should expect errors.";
                writable_preferences()->number_of_pages = std::max(writable_preferences()->number_of_pages, doc->numberOfPages());
            }
            else
                writable_preferences()->number_of_pages = doc->numberOfPages();
            connect(this, &Master::sendAction, doc, &PdfMaster::receiveAction);
            connect(doc, &PdfMaster::navigationSignal, this, &Master::navigateToPage, Qt::QueuedConnection);
            connect(this, &Master::navigationSignal, doc, &PdfMaster::distributeNavigationEvents, Qt::QueuedConnection);
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
            if (shift != 0)
                scene->setPageShift(shift);
            if (object.value("master").toBool())
                doc->getScenes().prepend(scene);
            else
                doc->getScenes().append(scene);
            connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
            connect(this, &Master::prepareNavigationSignal, scene, &SlideScene::prepareNavigationEvent);
        }
        else if (object.value("master").toBool())
            doc->getScenes().swapItemsAt(doc->getScenes().indexOf(scene), 0);
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
        connect(slide, &SlideView::sendKeyEvent, this, &Master::receiveKeyEvent);
        connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged);
        connect(scene, &SlideScene::clearViews, slide, &SlideView::clearBackground);
        widget = slide;
        break;
    }
    case OverviewType:
        widget = new ThumbnailWidget(parent);
        if (object.contains("columns"))
            static_cast<ThumbnailWidget*>(widget)->setColumns(object.value("columns").toInt(4));
        connect(static_cast<ThumbnailWidget*>(widget), &ThumbnailWidget::sendNavigationSignal, this, &Master::navigateToPage);
        if (object.value("overlays").toString() == "skip")
            static_cast<ThumbnailWidget*>(widget)->skipOverlays();
        break;
    case TOCType:
        widget = new TOCwidget(parent);
        connect(static_cast<TOCwidget*>(widget), &TOCwidget::sendNavigationSignal, this, &Master::navigateToPage);
        break;
    case NotesType:
        widget = new NotesWidget(parent);
        connect(this, &Master::navigationSignal, static_cast<NotesWidget*>(widget), &NotesWidget::pageChanged, Qt::QueuedConnection);
        static_cast<NotesWidget*>(widget)->zoomIn(object.value("zoom").toInt(10));
        if (object.contains("file"))
            static_cast<NotesWidget*>(widget)->load(object.value("file").toString());
        break;
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
                case QJsonValue::Object:
                {
                    const QJsonObject obj = row[j].toObject();
                    Tool *tool = NULL;
                    const BasicTool base_tool = string_to_tool.value(obj.value("tool").toString());
                    switch (base_tool)
                    {
                    case Pen:
                    {
                        const QColor color(obj.value("color").toString("black"));
                        const float width = obj.value("width").toDouble(2.);
                        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
                        tool = new DrawTool(Pen, AnyDevice, QPen(color, width, style, Qt::RoundCap, Qt::RoundJoin));
                        break;
                    }
                    case Highlighter:
                    {
                        const QColor color(obj.value("color").toString("yellow"));
                        const float width = obj.value("width").toDouble(20.);
                        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
                        tool = new DrawTool(Highlighter, AnyDevice, QPen(color, width, style, Qt::RoundCap, Qt::RoundJoin), QPainter::CompositionMode_Darken);
                        break;
                    }
                    case InvalidTool:
                        break;
                    default:
                        tool = new Tool(base_tool, AnyDevice);
                        break;
                    }
                    if (tool)
                        toolwidget->addToolButton(i, j, tool);
                    else
                        qWarning() << "Failed to create tool button" << obj.value("tool") << obj;
                    break;
                }
                default:
                    break;
                }
            }
        }
        connect(toolwidget, &ToolSelectorWidget::sendAction, this, &Master::handleAction);
        connect(toolwidget, &ToolSelectorWidget::sendTool, this, &Master::setTool);
        widget = toolwidget;
        break;
    }
    case SettingsType:
        widget = new SettingsWidget(parent);
        break;
    case ClockType:
        widget = new ClockWidget(parent);
        break;
    case TimerType:
        widget = new TimerWidget(parent);
        connect(this, &Master::sendAction, static_cast<TimerWidget*>(widget), &TimerWidget::handleAction);
        break;
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
        qWarning() << "Requested GUI type is not implemented (yet):" << object.value("type");
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
        auto it = preferences()->key_actions.constFind(key_code);
        while (it != preferences()->key_actions.cend())
        {
            debug_msg(DebugKeyInput) << "Global key action:" << it.value();
            debug_msg(DebugKeyInput) << "Cache:" << getTotalCache();
            handleAction(it.value());
            if ((++it).key() != static_cast<unsigned int>(event->key()))
                break;
        }
    }
    // Search tools in preferences for given key sequence.
    for (const auto tool : static_cast<const QList<const Tool*>>(preferences()->key_tools.values(key_code)))
    {
        if (tool)
            switch (tool->tool())
            {
            case Pen:
            case Highlighter:
                setTool(new DrawTool(*static_cast<const DrawTool*>(tool)));
                break;
            default:
                setTool(new Tool(*tool));
                break;
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
    case Quit:
        for (const auto window : qAsConst(windows))
            window->close();
        break;
    case ReloadFiles:
        for (const auto doc : qAsConst(documents))
            doc->loadDocument();
        break;
    default:
        emit sendAction(action);
    }
}

void Master::limitHistoryInvisible(const int page) const
{
    PathContainer *container;
    bool flexible_page_numbers = false;
    for (const auto doc : qAsConst(documents))
    {
        container = doc->pathContainer(page);
        if (container)
            container->clearHistory(preferences()->history_length_hidden_slides);
        container = doc->pathContainer(page | PagePart::LeftHalf);
        if (container)
            container->clearHistory(preferences()->history_length_hidden_slides);
        container = doc->pathContainer(page | PagePart::RightHalf);
        if (container)
            container->clearHistory(preferences()->history_length_hidden_slides);
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
    limitHistoryInvisible(preferences()->page);
    emit prepareNavigationSignal(page);
    for (auto window : windows)
        window->updateGeometry();
    writable_preferences()->page = page;
    emit navigationSignal(page);
}

void Master::setTool(Tool *tool) const noexcept
{
    if (!tool)
        return;
    debug_msg(DebugDrawing|DebugKeyInput) << "Set tool" << tool->tool() << tool->device();
    const int device = tool->device();
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
        else
            ++tool_it;
    }
    writable_preferences()->current_tools.insert(tool);

#ifdef QT_DEBUG
    if ((preferences()->log_level & DebugVerbose) && preferences()->log_level & DebugDrawing)
        for (const auto tool : preferences()->current_tools)
            qDebug() << "tool:" << tool->device() << tool->tool() << tool;
#endif
}
