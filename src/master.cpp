#include "src/master.h"
#include "src/pdfmaster.h"
#include "src/slidescene.h"

Master::~Master()
{
    for (const auto cache : qAsConst(caches))
    {
        QThread * const thread = cache->thread();
        delete cache;
        thread->quit();
        thread->deleteLater();
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
        QWidget* const widget = createWidget(obj, nullptr);
        if (widget)
            windows.append(widget);
    }

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
            return nullptr;
        }
    }
    QWidget *widget = nullptr;
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

        // only for testing:
        QPalette palette = widget->palette();
        palette.setColor(QPalette::Background, type == VBoxWidgetType ? Qt::red : Qt::yellow);
        widget->setPalette(palette);

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

        // only for testing:
        QPalette palette = stackwidget->palette();
        palette.setColor(QPalette::Background, Qt::magenta);
        stackwidget->setPalette(palette);

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
    case TabedWidgetType:
    {
        QTabWidget *tabwidget = new QTabWidget(parent);
        tabwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        tabwidget->setTabPosition(string_to_tab_widget_orientation.value(object.value("orientation").toString()));

        // only for testing:
        QPalette palette = tabwidget->palette();
        palette.setColor(QPalette::Background, Qt::cyan);
        tabwidget->setPalette(palette);

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
            tabwidget->addTab(newwidget, obj.value("title").toString());
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
        // Usually "file" will be "presentation" or "notes". These are mapped to
        // filenames by this->files.
        QString file = object.value("file").toString();
        file = preferences().file_alias.value(file.isNull() ? "presentation" : file, file);
        if (!QFile::exists(file))
        {
            qCritical() << "Could not load PDF file: Does not exist." << file;
            return nullptr;
        }
        const QString page_part_str = object.value("page part").toString().toLower();
        PagePart page_part = FullPage;
        if (page_part_str == "left")
            page_part = LeftHalf;
        else if (page_part_str == "right")
            page_part = RightHalf;

        // Check whether the PDF has been loaded already, load it if necessary.
        PdfMaster *doc = nullptr;
        SlideScene *scene = nullptr;
        for (auto docit = documents.cbegin(); docit != documents.cend(); ++docit)
        {
            if ((*docit)->getFilename() == file)
            {
                doc = *docit;
                if (preferences().page_part_threshold > 0.)
                {
                    const QSizeF reference = doc->getPageSize(0);
                    if (reference.width() < preferences().page_part_threshold * reference.height())
                        page_part = FullPage;
                }
                break;
            }
        }
        if (doc == nullptr)
        {
            doc = new PdfMaster(file);
            if (writable_preferences().number_of_pages && writable_preferences().number_of_pages != doc->numberOfPages())
            {
                qWarning() << "Loaded PDF files with different numbers of pages. You should expect errors.";
                writable_preferences().number_of_pages = std::max(writable_preferences().number_of_pages, doc->numberOfPages());
            }
            else
                writable_preferences().number_of_pages = doc->numberOfPages();
            connect(this, &Master::sendAction, doc, &PdfMaster::receiveAction);
            connect(doc, &PdfMaster::nagivationSignal, this, &Master::navigationSignal);
            connect(this, &Master::navigationSignal, doc, &PdfMaster::distributeNavigationEvents);
            if (preferences().page_part_threshold > 0.)
            {
                const QSizeF reference = doc->getPageSize(0);
                if (reference.width() < preferences().page_part_threshold * reference.height())
                    page_part = FullPage;
            }
            if (object.value("master").toBool())
                documents.prepend(doc);
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
        if (scene == nullptr)
        {
            scene = new SlideScene(doc, page_part, parent);
            if (shift != 0)
                scene->setPageShift(shift);
            if (object.value("master").toBool())
                doc->getScenes().prepend(scene);
            else
                doc->getScenes().append(scene);
            connect(this, &Master::sendAction, scene, &SlideScene::receiveAction);
        }
        else if (object.value("master").toBool())
            doc->getScenes().swapItemsAt(doc->getScenes().indexOf(scene), 0);
        // TODO: read other properties from config

        // Get or create cache object.
        PixCache *pixcache = nullptr;
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
            pixcache = caches.value(cache_hash, nullptr);
        }
        // If necessary, create a new PixCache object an store it in caches.
        if (pixcache == nullptr)
        {
            // Read number of threads from GUI config.
            const int threads = object.value("threads").toInt(1);
            // Create the PixCache object.
            pixcache = new PixCache(scene->getPdfMaster()->getDocument(), threads, page_part);
            // Move the PixCache object to an own thread.
            pixcache->moveToThread(new QThread());
            // Make sure that pixcache is initialized when the thread is started.
            connect(pixcache->thread(), &QThread::started, pixcache, &PixCache::init);
            connect(this, &Master::navigationSignal, pixcache, &PixCache::pageNumberChanged);
            // Set maximum number of pages in cache from settings.
            pixcache->setMaxNumber(preferences().max_cache_pages);
            // Start the thread.
            pixcache->thread()->start();
            // Keep the new pixcache in caches.
            caches[cache_hash] = pixcache;
        }

        // Create slide view.
        SlideView *slide = new SlideView(scene, pixcache, parent);
        connect(slide, &SlideView::sendKeyEvent, this, &Master::receiveKeyEvent);
        connect(scene, &SlideScene::navigationToViews, slide, &SlideView::pageChanged);
        widget = slide;
    }
    case OverviewType:
        break;
    case TOCType:
        break;
    case NotesType:
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
                    Tool *tool = nullptr;
                    const BasicTool base_tool = string_to_tool.value(obj.value("tool").toString());
                    switch (base_tool)
                    {
                    case Pen:
                    {
                        const QColor color(obj.value("color").toString("black"));
                        const float width = obj.value("width").toDouble(2.);
                        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
                        tool = new DrawTool(Pen, AnyDevice, QPen(color, width, style, Qt::RoundCap));
                        break;
                    }
                    case Highlighter:
                    {
                        const QColor color(obj.value("color").toString("yellow"));
                        const float width = obj.value("width").toDouble(20.);
                        const Qt::PenStyle style = string_to_pen_style.value(obj.value("style").toString(), Qt::SolidLine);
                        tool = new DrawTool(Highlighter, AnyDevice, QPen(color, width, style, Qt::RoundCap), QPainter::CompositionMode_Darken);
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
        connect(this, &Master::navigationSignal, static_cast<SlideNumberWidget*>(widget), &SlideNumberWidget::updateText);
        break;
    case SlideLabelType:
        break;
    case GuiWidget::InvalidType:
        qWarning() << "Ignoring entry in GUI config with invalid type:" << object.value("type");
    }
    if (!widget)
        qWarning() << "Requested GUI type is not implemented (yet):" << object.value("type");
    // Add keyboard shortcut.
    else if (object.contains("keys"))
    {
        const QKeySequence seq(object.value("keys").toString());
        if (seq.isEmpty())
            qWarning() << "Unknown key sequence in config:" << object.value("keys");
        else
            shortcuts[seq[0] + seq[1] + seq[2] + seq[3]] = widget;
    }
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
        qDebug() << "Key action:" << widget << event << (event->key() | (event->modifiers() & ~Qt::KeypadModifier));
        if (widget)
        {
            widget->show();
            QStackedWidget *stackwidget = dynamic_cast<QStackedWidget*>(widget->parentWidget());
            qDebug() << widget << stackwidget << widget->parentWidget();
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
        auto it = preferences().key_actions.constFind(key_code);
        while (it != preferences().key_actions.cend())
        {
#ifdef DEBUG_KEY_ACTIONS
            qDebug() << "Global key action:" << it.value();
            qDebug() << "Cache:" << getTotalCache();
#endif
            handleAction(it.value());
            if ((++it).key() != static_cast<unsigned int>(event->key()))
                break;
        }
    }
    // Search tools in preferences for given key sequence.
    for (const auto tool : static_cast<const QList<const Tool*>>(preferences().key_tools.values(key_code)))
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
        navigateToPage(preferences().page);
        break;
    case NextPage:
        navigateToPage(preferences().page + 1);
        break;
    case PreviousPage:
        navigateToPage(preferences().page - 1);
        break;
    case NextSkippingOverlays:
        navigateToPage(documents.first()->overlaysShifted(preferences().page, 1 | FirstOverlay));
        break;
    case PreviousSkippingOverlays:
        navigateToPage(documents.first()->overlaysShifted(preferences().page, -1 & ~FirstOverlay));
        break;
    case FirstPage:
        navigateToPage(0);
        break;
    case LastPage:
        navigateToPage(preferences().number_of_pages - 1);
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
            container->clearHistory(preferences().history_length_hidden_slides);
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
    if (preferences().max_memory < 0)
        return;
    float scale = 0.;
    for (const auto cache : qAsConst(caches))
        scale += cache->getPixels();
    if (scale <= 0)
        return;
    scale = preferences().max_memory / scale;
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

void Master::navigateToPage(const int page)
{
    if (page < 0 || page >= preferences().number_of_pages)
        return;
    limitHistoryInvisible(preferences().page | preferences().page_part);
    writable_preferences().page = page;
    emit navigationSignal(page);
}

void Master::setTool(Tool *tool) const noexcept
{
    if (!tool)
        return;
    qDebug() << "Set tool" << tool->tool() << tool->device();
    const int device = tool->device();
    int newdevice;
    for (auto tool_it = preferences().current_tools.cbegin(); tool_it != preferences().current_tools.cend();)
    {
        if ((*tool_it)->device() & device)
        {
            newdevice = (*tool_it)->device() & ~device;
            if (newdevice)
                (*tool_it++)->setDevice(newdevice);
            else
            {
                delete *tool_it;
                tool_it = static_cast<QSet<Tool*>::const_iterator>(writable_preferences().current_tools.erase(tool_it));
            }
        }
        else
            ++tool_it;
    }
    writable_preferences().current_tools.insert(tool);

    // debug
    //for (const auto tool : preferences().current_tools)
    //    qDebug() << "tool:" << tool->device() << tool->tool() << tool;
}
